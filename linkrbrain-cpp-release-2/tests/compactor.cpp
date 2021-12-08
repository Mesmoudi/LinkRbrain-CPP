#include "Exceptions/Exception.hpp"
#include "Logging/Logger.hpp"
auto& logger = Logging::get_logger();

#include "LinkRbrain/Compaction/MultiplyPointCompactor.hpp"
#include "LinkRbrain/Compaction/IndexedPointCompactor.hpp"
#include "LinkRbrain/Compaction/BitwisePointCompactor.hpp"
#include "LinkRbrain/Compaction/Manager.hpp"
#include "Buffering/Writing/FileWriter.hpp"
#include "Generators/Random.hpp"
// only for real points extraction
#include "LinkRbrain/Models/Dataset.hpp"
#include "LinkRbrain/Parsing/FunctionsV1Parser.hpp"
#include "LinkRbrain/Parsing/GenomicsV1Parser.hpp"

using namespace LinkRbrain::Models;
using namespace LinkRbrain::Compaction;


static const bool use_random_points = false;
static int N = 1e7;
static double resolution = 2.0;


void randomize_point(WeightedPoint& point, const Extrema& extrema) {
    point.x = Generators::Random::generate_number<double>(extrema.min.x, extrema.max.x, resolution);
    point.y = Generators::Random::generate_number<double>(extrema.min.y, extrema.max.y, resolution);
    point.z = Generators::Random::generate_number<double>(extrema.min.z, extrema.max.z, resolution);
}
const bool check(const WeightedPoint& p1, const WeightedPoint& p2) {
    if (abs(p1.x - p2.x) > resolution) return false;
    if (abs(p1.y - p2.y) > resolution) return false;
    if (abs(p1.z - p2.z) > resolution) return false;
    return true;
}


int main(int argc, char const *argv[]) {
    Extrema extrema;
    std::vector<WeightedPoint> points;
    if (use_random_points) {
        WeightedPoint point_min(-68, -108, -70, 0);
        WeightedPoint point_max(-68, -108, +78, 0);
        // WeightedPoint point_min(-68, -108, -70, 0);
        // WeightedPoint point_max(+70,  +68, +78, 0);
        extrema.integrate(point_min);
        extrema.integrate(point_max);
        points.resize(N);
        points[0] = point_min;
        points[1] = point_max;
        for (int i=2; i<N; ++i) {
            randomize_point(points[i], extrema);
        }
        logger.message("Generated", points.size(), "points");
    } else {
        const std::filesystem::path path = "var/lib/linkrbrain/datasets/genes/data";
        Buffering::Reading::FileReader reader(path, Buffering::Format::Binary);
        LinkRbrain::Models::Dataset dataset("foo");
        reader >> dataset;
        logger.notice("Loaded dataset from " + path.native());
        // LinkRbrain::Models::Dataset dataset("genes");
        // LinkRbrain::Parsing::GenomicsV1Parser::parse("data/old/genes/", dataset);
        // LinkRbrain::Models::Dataset dataset("functions");
        // LinkRbrain::Parsing::FunctionsV1Parser::parse("data/old/fichiers_barycenter_New/", dataset, "barycenter_t_", ".txt");
        for (const LinkRbrain::Models::Group& group : dataset.get_groups()) {
            for (const LinkRbrain::Models::WeightedPoint& point : group.get_points()) {
                points.push_back({point.x, point.y, point.z, 0.0});
                extrema.integrate(point);
            }
        }
        N = points.size();
        logger.debug("Extracted", points.size(), "points");
    }
    //
    std::vector<std::pair<std::string, PointCompactor*>> compactors = {
        {"bitwise", new BitwisePointCompactor(extrema, resolution)},
        {"multiply", new MultiplyPointCompactor(extrema, resolution)},
        {"index", new IndexedPointCompactor(extrema, resolution)},
    };
    logger.notice("Instanciated", compactors.size(), "compactors\n");
    //
    for (auto& [name, compactor] : compactors) {
        logger.debug("Using", name, "compactor, theoretical maximum is", compactor->get_maximum_compacted_coordinates());
        int maximum = 0;
        for (int i=0; i<N; ++i) {
            const auto compacted = compactor->compact_coordinates(points[i]);
            if (compacted > maximum) {
                maximum = compacted;
            }
        }
        logger.notice("Compacted", points.size(), "using", name, "... maximum is", maximum);
        std::vector<uint64_t> compacted;
        compacted.resize(N);
        for (int i=0; i<N; ++i) {
            compacted[i] = compactor->compact_coordinates(points[i]);
        }
        logger.debug("Generated", points.size(), "compacted using", name);
        for (int i=0; i<N; ++i) {
            const auto decompacted = compactor->decompact_coordinates(compacted[i]);
            if (!check(points[i], decompacted)) {
                logger.warning("Expected:", points[i].x, points[i].y, points[i].z, points[i].weight);
                logger.warning("Got:", decompacted.x, decompacted.y, decompacted.z, decompacted.weight);
                except("Error while decompacting item", i);
            }
        }
        logger.debug("Compared", points.size(), "compacted with their original using", name);
        logger.notice("Checked bijectivity on", points.size(), "points using", name);
        std::filesystem::path path = "/tmp";
        path /= "compactor-" + name;
        LinkRbrain::Compaction::Manager::save(compactor, path);
        logger.notice("Saved " + name + " compactor to " + path.native());
        auto compactor2 = LinkRbrain::Compaction::Manager::load(path);
        logger.debug("Loaded " + compactor2->get_type_name() + " compactor from " + path.native());
        for (int i=0; i<N; ++i) {
            const auto compacted2 = compactor2->compact_coordinates(points[i]);
            if (compacted[i] != compacted2) {
                except("Loaded compactor made a mistake while compacting");
            }
            const auto decompacted = compactor2->decompact_coordinates(compacted[i]);
            if (!check(points[i], decompacted)) {
                except("Loaded compactor made a mistake while decompacting");
            }
            compacted[i] = compactor2->compact_coordinates(points[i]);
        }
        logger.debug("Checked data using loaded compactor");
        logger.notice("Successfully loaded compactor from " + path.native());
        logger.message("Completed tests for", name, "\n");
    }
    return 0;
}
