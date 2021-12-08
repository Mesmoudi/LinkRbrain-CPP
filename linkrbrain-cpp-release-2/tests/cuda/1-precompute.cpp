#include "LinkRbrain/Models/Dataset.hpp"
#include "Buffering/Reading/FileReader.hpp"
#include "LinkRbrain/Scoring/Correlator.hpp"

#include "Logging/Loggers.hpp"
auto& logger = Logging::get_logger();


static const int device = 0;
static const size_t threads = 1 << 8;
static const double resolution = 2.0;
static const double diameter = 10.0;


void CUDA_precomputing_start(int device, size_t threads);
void CUDA_precomputing_set_groups(const size_t groups_count, const std::vector<Types::Point<double>>& groups_points, const std::vector<size_t>& groups_offsets);
void CUDA_precomputing_precompute_sphere(std::vector<double>& precomputed, const Types::Point<double> center, const double diameter);
void CUDA_precomputing_finish();


int main(int argc, char const *argv[]) {

    // load dataset
    Buffering::Reading::FileReader reader("var/lib/linkrbrain/datasets/functions/data", Buffering::Format::Binary);
    LinkRbrain::Models::Dataset<double> original_dataset("functions");
    reader >> original_dataset;
    logger.notice("Loaded dataset");
    size_t points_count = 0;
    for (auto& group : original_dataset.get_groups()) {
        points_count += group.get_points().size();
    }
    logger.debug("Dataset has", original_dataset.get_groups().size(), "groups and", points_count, "points");

    // instanciate correlator
    LinkRbrain::Scoring::Correlator<double> correlator(
        original_dataset,
        resolution,
        LinkRbrain::Scoring::Scorer::Mode::Sphere,
        diameter
    );
    const LinkRbrain::Models::Dataset<double>& dataset = correlator.get_dataset();
    logger.message("Instanciated correlator");

    // flatten groups
    std::vector<Types::Point<double>> points;
    std::vector<size_t> offsets = {0};
    for (size_t i=0, n=dataset.get_groups().size(); i<n; ++i) {
        const auto& group_points = dataset.get_group(i).get_points();
        points.insert(points.end(), group_points.begin(), group_points.end());
        offsets.push_back(points.size());
    }
    logger.notice("Flattened dataset with", points.size(), "points and", offsets.size(), "offsets");
    logger.debug("Theoretical size:", sizeof(Types::Point<double>) * points.size());
    logger.debug("Real size:", sizeof(Types::Point<double>) + ((const char*)(&points.back()) - (const char*)(&points.front())));
    std::vector<double> precomputed;
    logger.message("Data is ready");

    // do the thing
    auto cache = LinkRbrain::Scoring::Caching::MemoryScorerCache<double>(dataset.get_groups());
    logger.notice("Instanciated cache");
    CUDA_precomputing_start(device, threads);
    logger.notice("Instanciated CUDA precomputer");
    CUDA_precomputing_set_groups(
        dataset.get_groups().size(),
        points,
        offsets
    );
    logger.notice("Initialized CUDA precomputer with groups data");
    precomputed.resize(dataset.get_groups().size());
    logger.debug("Initialized precomputed vector to", precomputed.size(), "empty elements");
    size_t count = 0;
    for (const auto& item : correlator.get_density_map()) {
        if (! * item.value) continue;
        CUDA_precomputing_precompute_sphere(precomputed, item.coordinates, diameter);
        cache.integrate(item.index, precomputed, true);
        if (++count % 100 == 0) {
            std::cout << item.index << " / " << correlator.get_density_map().get_size() << '\r';
        }
    }
    logger.message("Iterated over all groups to make cache");
    CUDA_precomputing_finish();
    logger.debug("Stop CUDA precomputer");

    // the end!
    return 0;
}
