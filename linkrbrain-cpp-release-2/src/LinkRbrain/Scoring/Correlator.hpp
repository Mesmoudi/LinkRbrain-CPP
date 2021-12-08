#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CORRELATOR_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CORRELATOR_HPP


#include "../Models/Dataset.hpp"
#include "../Graph/Graph.hpp"
#include "./Scorer.hpp"
#include "./ScoredGroupList.hpp"
#include "./Caching/Manager.hpp"
#include "Types/NumberNature.hpp"
#include "Conversion/Binary.hpp"

#include "Logging/Loggable.hpp"

#include <thread>
#include <fstream>
#include <filesystem>
#include <unordered_map>


#ifdef USE_CUDA_OPTIMISATION
void CUDA_precomputing_start(int device, size_t threads);
void CUDA_precomputing_set_groups(const size_t groups_count, const std::vector<Types::Point<double>>& groups_points, const std::vector<size_t>& groups_offsets);
void CUDA_precomputing_precompute_sphere(std::vector<double>& precomputed, const Types::Point<double> center, const double diameter);
void CUDA_precomputing_finish();
#endif // USE_CUDA_OPTIMISATION


namespace LinkRbrain::Scoring {


    template <typename T>
    class Correlator : public Logging::Loggable {
    public:

        enum Status : uint32_t {
            None = 0x00,
            Naive = 0x01,
            NormalizingWithin = 0x11,
            NormalizedWithin = 0x12,
            ComputingDensityMap = 0x13,
            ComputedDensityMap = 0x14,
            NormalizingDensity = 0x15,
            NormalizedDensity = 0x16,
            NormalizingWithin2 = 0x17,
            NormalizedAll = 0x18,
            CachingPoints = 0x20,
            CachedPoints = 0x21,
            CachingGroups = 0x40,
            CachedGroups = 0x41,
        };

        Correlator(const LinkRbrain::Models::Dataset<T>& dataset, const T& resolution, const Scorer::Mode& mode, const T diameter) :
            _original_dataset(dataset),
            _original_dataset_hash(dataset.compute_hash()),
            _dataset(dataset),
            _scorer(mode, diameter),
            _density_map(compute_density_map_extrema(), resolution),
            _status(Naive),
            _progress(0)
        {
            normalize();
        }

        Correlator(const LinkRbrain::Models::Dataset<T>& dataset, const std::filesystem::path& path) :
            _original_dataset(dataset)
        {
            // load members
            Conversion::Binary::parse_file(path / "normalized_dataset", _dataset);
            Conversion::Binary::parse_file(path / "density_map", _density_map);
            // open configuration file
            std::ifstream buffer(path / "config");
            // check template type
            Types::NumberNature number_nature = {.type=Types::NumberNature::Other};
            Conversion::Binary::straight_parse(buffer, number_nature);
            if (number_nature != Types::NumberNatureOf<T>) {
                except("Number nature is different in template parameter (" + Types::NumberNatureOf<T>.get_full_name() + ") than in loaded file (" + number_nature.get_full_name() + ").");
            }
            // check original dataset hash
            Conversion::Binary::parse(buffer, _original_dataset_hash);
            if (_original_dataset_hash != _original_dataset.compute_hash()) {
                except("Dataset hash does not match the one found in loaded correlator.");
            }
            // retrieve status
            Conversion::Binary::parse(buffer, _status);
            Conversion::Binary::parse(buffer, _progress);
            // configure scorer
            Scorer::Mode mode;
            T diameter;
            Conversion::Binary::parse(buffer, mode);
            Conversion::Binary::parse(buffer, diameter);
            _scorer.set_mode(mode);
            _scorer.set_diameter(diameter);
            get_logger().debug("Loaded scorer in", _scorer.get_mode_name(), "mode with a diameter of", diameter);
            // normalize according to existing status
            get_logger().notice("Loaded dataset from", path.native(), "with status", get_progress_string());
            normalize(false);
        }

        const size_t get_dataset_group_index(const Models::Group<T>& group, const bool is_recursing=false) {
            const auto it = _groups_indexes.find(&group);
            if (it == _groups_indexes.end()) {
                if (is_recursing) {
                    throw Exceptions::Exception("Could not retrieve index of group: " + group.get_label());
                } else {
                    _groups_indexes.clear();
                    const auto& groups = _dataset.get_groups();
                    for (size_t group_index = 0; group_index < groups.size(); group_index++) {
                        _groups_indexes.insert({&(groups[group_index]), group_index});
                    }
                    return get_dataset_group_index(group, true);
                }
            }
            return it->second;
        }

        void save_config(const std::filesystem::path& path) {
            std::ofstream buffer(path / "config");
            Conversion::Binary::straight_serialize(buffer, Types::NumberNatureOf<T>);
            Conversion::Binary::serialize(buffer, _original_dataset_hash);
            Conversion::Binary::serialize(buffer, _status);
            Conversion::Binary::serialize(buffer, _progress);
            Conversion::Binary::serialize(buffer, _scorer.get_mode());
            Conversion::Binary::serialize(buffer, _scorer.get_diameter());
            get_logger().debug("Saved configuration to", (path / "config").native());
        }
        void save_normalized_dataset(const std::filesystem::path& path) {
            std::ofstream buffer(path / "normalized_dataset");
            Conversion::Binary::serialize(buffer, _dataset);
            get_logger().debug("Saved normalized dataset to", (path / "normalized_dataset").native());
        }
        void save_density_map(const std::filesystem::path& path) {
            std::ofstream buffer(path / "density_map");
            Conversion::Binary::serialize(buffer, _density_map);
            get_logger().debug("Saved density map to", (path / "density_map").native());
        }
        void save(const std::filesystem::path& path) {
            std::filesystem::create_directories(path);
            save_config(path);
            save_normalized_dataset(path);
            save_density_map(path);
            get_logger().notice("Saved to", path.native());
        }

        const T score(std::vector<Types::Point<T>> points1, std::vector<Types::Point<T>> points2) {
            normalize_between_groups(points1);
            normalize_group_within(points1);
            normalize_between_groups(points2);
            normalize_group_within(points2);
            return _scorer.score(points1, points2);
        }

        const ScoredGroupList<T> correlate(std::vector<std::vector<Types::Point<T>>> query_groups_points, const bool sort=true, const size_t limit=-1, const bool force_uncached=false, const bool use_interpolation=false) {
            // normalize & compute
            for (std::vector<Types::Point<T>>& query_group_points : query_groups_points) {
                normalize_between_groups(query_group_points);
                normalize_group_within(query_group_points);
            }
            // instanciate result
            ScoredGroupList<T> result(_dataset.get_groups(), query_groups_points.size());
            // compute scores for each query group
            const bool uncached = (_status < CachedPoints || force_uncached);
            for (size_t query_group_index = 0; query_group_index < query_groups_points.size(); query_group_index++) {
                if (uncached) {
                    correlate_uncached(result, query_group_index, query_groups_points[query_group_index]);
                } else {
                    correlate_cached(result, query_group_index, query_groups_points[query_group_index], use_interpolation);
                }
            }
            // compute overall scores
            for (ScoredGroup<T>& scored_group : result) {
                scored_group.overall_score = _scorer.compute_overall_score(scored_group.scores);
            }
            // sort if nececessary
            if (sort) {
                result.sort(limit);
                get_logger().debug("Sorted correlation result");
            }
            return result;
        }

        const std::vector<T> compute_group_scores(const Models::Group<T>& group, const bool force_uncached=false) {
            const size_t group_index = get_dataset_group_index(group);
            return (!_groups_cache || _status < CachedGroups || force_uncached)
                ? compute_group_scores_uncached(group_index)
                : compute_group_scores_cached(group_index);
        }
        const std::vector<T> compute_group_scores_cached(const size_t group_index) {
            return _groups_cache->get_score_map(group_index);
        }
        const std::vector<T> compute_group_scores_uncached(const size_t group) {
            throw Exceptions::Exception("Not implemented: Correlator::compute_group_scores_uncached");
        }

        // caching

        enum ComputingMode {
            Basic = 0,
            Multithreading = 1,
            GPU = 2,
        };

        void load_points_cache(const LinkRbrain::Scoring::Caching::Type& caching_type, const std::filesystem::path& path) {
            _points_cache.reset(
                Caching::Manager::make<T>(
                    caching_type,
                    _dataset.get_groups(),
                    path
                )
            );
            get_logger().debug("Instanciated correlator points cache object by loading ", path);
        }
        void load_groups_cache(const LinkRbrain::Scoring::Caching::Type& caching_type, const std::filesystem::path& path) {
            _groups_cache.reset(
                Caching::Manager::make<T>(
                    caching_type,
                    _dataset.get_groups(),
                    path
                )
            );
            get_logger().debug("Instanciated correlator groups cache object by loading ", path);
        }

        void compute_points_cache(const LinkRbrain::Scoring::Caching::Type& caching_type, const std::filesystem::path& path=".", ComputingMode computing_mode=Basic, const size_t n_threads=std::thread::hardware_concurrency()) {
            #ifndef USE_CUDA_OPTIMISATION
            if (computing_mode == GPU) {
                get_logger().warning("Computing mode is set to GPU, but program has not been compiled with CUDA");
                computing_mode = Basic;
            }
            #endif
            // prepare
            const auto& groups = _dataset.get_groups();
            const size_t start_index = (_status == CachingPoints) ? _progress : 0;
            _status = CachingPoints;
            _points_cache.reset(
                Caching::Manager::make<T>(caching_type, groups, path)
            );
            get_logger().debug("Instanciated cache object for computing");
            if (start_index == 0) {
                _points_cache->clear();
                get_logger().debug("Cleared cache");
            }
            std::vector<T> scores;
            scores.resize(groups.size());
            size_t count = 0;
            #ifdef USE_CUDA_OPTIMISATION
            if (computing_mode == GPU) {
                CUDA_precomputing_start(0, n_threads);
                get_logger().debug("Instanciated CUDA precomputer");
                std::vector<Types::Point<double>> groups_points;
                std::vector<size_t> groups_offsets = {0};
                for (size_t i=0, n=_dataset.get_groups().size(); i<n; ++i) {
                    const auto& group_points = _dataset.get_group(i).get_points();
                    groups_points.insert(groups_points.end(), group_points.begin(), group_points.end());
                    groups_offsets.push_back(groups_points.size());
                }
                get_logger().debug("Flattened groups data for CUDA");
                CUDA_precomputing_set_groups(
                    _dataset.get_groups().size(),
                    groups_points,
                    groups_offsets
                );
                get_logger().debug("Initialized CUDA precomputer with groups data");
            }
            #endif // USE_CUDA_OPTIMISATION
            double t0 = Logging::Logger::get_millitime();
            size_t index0 = start_index;
            const int display_rate = 20;
            get_logger().debug("Start computation from point index", start_index);
            for (auto& item : _density_map) {
                if (! * item.value || item.index < start_index) {
                    continue;
                }
                _progress = item.index;
                switch (computing_mode) {
                    case Basic:
                        for (size_t i=0, n=groups.size(); i<n; ++i) {
                            scores[i] = _scorer.score(item.coordinates, groups[i].get_points());
                        }
                    break;
                    case Multithreading: {
                        const size_t delta = ceil((T) groups.size() / (T) (n_threads - 0));
                        size_t i_min = 0;
                        size_t i_max = delta;
                        std::vector<std::thread*> threads;
                        for (int t=0; t<n_threads; ++t) {
                            threads.push_back(
                                new std::thread(precomputing_thread,
                                    std::ref(scores), std::ref(_scorer),
                                    item.coordinates, std::ref(groups),
                                    i_min, std::min(i_max, groups.size())
                                )
                            );
                            i_min += delta;
                            i_max += delta;
                        }
                        for (std::thread* thread : threads) {
                            thread->join();
                        }
                    } break;
                    case GPU:
                        #ifdef USE_CUDA_OPTIMISATION
                        CUDA_precomputing_precompute_sphere(scores, item.coordinates, _scorer.get_diameter());
                        #endif // USE_CUDA_OPTIMISATION
                        break;
                }
                _points_cache->integrate(item.index, scores, true);
                if (++count % display_rate == 0) {
                    const double dt = Logging::Logger::get_millitime() - t0;
                    const size_t rate = (double)display_rate / dt;
                    const size_t index_rate = (double)(item.index - index0) / dt;
                    index0 = item.index;
                    std::cout << item.index << " / " << _density_map.get_size() << " computed points, " << index_rate << " (" << rate << ")" << " points/second    " << '\r';
                    t0 = Logging::Logger::get_millitime();
                }
            }
            // the end!
            _status = CachedPoints;
            get_logger().notice("Computed cache");
            #ifdef USE_CUDA_OPTIMISATION
            if (computing_mode == GPU) {
                CUDA_precomputing_finish();
                get_logger().debug("Kill CUDA precomputer");
            }
            #endif // USE_CUDA_OPTIMISATION
        }
        void compute_groups_cache(const LinkRbrain::Scoring::Caching::Type& caching_type, const std::filesystem::path& path="") {
            _status = CachingGroups;
            // those are the groups
            const auto& groups = _dataset.get_groups();
            // instanciate cache
            _groups_cache.reset(
                Caching::Manager::make<T>(caching_type, groups, path)
            );
            // for each of the group, compute all correlations
            for (size_t i = 0; i < groups.size(); i++) {
                const auto& group = groups[i];
                // inform command-line user about current status
                std::cout << '\r' << std::string(64, ' ');
                std::cout << "\rComputing group " << (i+1) << " / " << groups.size() << ": " << group.get_label();
                std::cout.flush();
                // correlate group with all the others & integrate into cache
                const std::vector<std::vector<Types::Point<double>>> points{{group.get_points()}};
                const ScoredGroupList<T> correlations = correlate(points, false);
                std::vector<T> scores;
                for (const auto& correlation : correlations) {
                    scores.push_back(correlation.scores[0]);
                }
                _groups_cache->integrate(i, scores, true);
            }
            // the end!
            std::cout << '\r' << std::string(64, ' ') << "\rComputed all " << groups.size() << " groups.\n";
            std::cout.flush();
            _status = CachedGroups;
        }

        // getters

        const Status& get_status() const {
            return _status;
        }
        static const std::string get_status_name(const Status& status) {
            switch (status) {
                case Naive:
                    return "Naive";
                case NormalizingWithin:
                    return "NormalizingWithin";
                case NormalizedWithin:
                    return "NormalizedWithin";
                case ComputingDensityMap:
                    return "ComputingDensityMap";
                case ComputedDensityMap:
                    return "ComputedDensityMap";
                case NormalizingDensity:
                    return "NormalizingDensity";
                case NormalizedDensity:
                    return "NormalizedDensity";
                case NormalizingWithin2:
                    return "NormalizingWithin2";
                case NormalizedAll:
                    return "NormalizedAll";
                case CachingPoints:
                    return "CachingPoints";
                case CachedPoints:
                    return "CachedPoints";
                case CachingGroups:
                    return "CachingGroups";
                case CachedGroups:
                    return "CachedGroups";
                default:
                    return "?";
            }
        }
        const std::string get_status_name() const {
            return get_status_name(_status);
        }
        const LinkRbrain::Models::Dataset<T>& get_dataset() const {
            return _dataset;
        }
        const Types::Array3D<T>& get_density_map() const {
            return _density_map;
        }
        const size_t get_progress() const {
            return _progress;
        }
        const std::string get_progress_string() const {
            return get_status_name() + "|" + std::to_string(_progress);
        }

    protected:

        virtual const std::string get_logger_name() {
            return "Correlator";
        }

    private:

        // precomputing thread

        static void precomputing_thread(std::vector<T>& scores, Scorer& scorer, const Types::Point<T> center, const std::vector<LinkRbrain::Models::Group<T>>& groups, const size_t i_min, const size_t i_max) {
            for (size_t i=i_min; i<i_max; ++i) {
                scores[i] = scorer.score(center, groups[i].get_points());
            }
        }

        // correlation itself

        void correlate_uncached(ScoredGroupList<T>& result, const size_t query_group_index, const std::vector<Types::Point<T>>& query_group_points) {
            for (ScoredGroup<T>& item : result) {
                const T score
                    = _scorer.score(item.group.get_points(), query_group_points);
                if (std::isnan(score)) {
                    get_logger().warning("Score for `", item.group.get_label(), "` is NaN");
                } else {
                    item.scores[query_group_index] = score;
                }
            }
            get_logger().debug("Correlated points without using cache");
        }
        void correlate_cached(ScoredGroupList<T>& result, const size_t query_group_index, const std::vector<Types::Point<T>>& query_group_points, const bool use_interpolation) {
            if (_points_cache.get() == NULL) {
                except("Cache is not set");
            }
            //
            if (use_interpolation) {
                // compute result using interpolation
                for (const Types::Point<T>& point : query_group_points) {
                    for (const auto& [point_index, coefficient] : _density_map.compute_indices(point.x, point.y, point.z)) {
                        if (coefficient == static_cast<T>(0.0)) {
                            continue;
                        }
                        result.increment_scores(
                            query_group_index,
                            _points_cache->get_score_map(point_index),
                            coefficient * std::sqrt(point.weight)
                        );
                    }
                }
            } else {
                // compute result using grid
                for (const Types::Point<T>& point : query_group_points) {
                    const size_t point_index = _density_map.compute_index(point.x, point.y, point.z);
                    result.increment_scores(
                        query_group_index,
                        _points_cache->get_score_map(point_index),
                        (point.weight >= 0) ? std::sqrt(point.weight) : -std::sqrt(-point.weight)
                    );
                }
            }
            //
            get_logger().debug("Correlated points using cache for query group #", query_group_index);
        }

        // density map

        const Types::PointExtrema<T> compute_density_map_extrema() {
            Types::PointExtrema<T> extrema = _dataset.compute_extrema();
            _scorer.inflate(extrema);
            return extrema;
        }
        void compute_density_map() {
            const auto& groups = _dataset.get_groups();
            for (_progress=0; _progress<groups.size(); ++_progress) {
                for (const auto& point : groups[_progress].get_points()) {
                    _scorer.project(_density_map, point);
                }
            }
            get_logger().notice("Computed density map");
        }

        // normalization

        void normalize(const bool force = false) {
            if (force || _status < NormalizedWithin) {
                if (_status != NormalizingWithin) {
                    _progress = 0;
                }
                _status = NormalizingWithin;
                normalize_groups_within();
                _status = NormalizedWithin;
            }
            if (force || _status < ComputedDensityMap) {
                if (_status != ComputingDensityMap) {
                    _progress = 0;
                }
                _status = ComputingDensityMap;
                compute_density_map();
                _status = ComputedDensityMap;
            }
            if (force || _status < NormalizedDensity) {
                if (_status != NormalizingDensity) {
                    _progress = 0;
                }
                _status = NormalizingDensity;
                normalize_between_groups();
                _status = NormalizedDensity;
            }
            if (force || _status < NormalizedAll) {
                if (_status != NormalizingWithin2) {
                    _progress = 0;
                }
                _status = NormalizingWithin2;
                normalize_groups_within();
                _status = NormalizedAll;
            }
        }

        void normalize_group_within(std::vector<Types::Point<T>>& points) {
            const T autoscore = _scorer.autoscore(points);
            if (autoscore == static_cast<T>(0.)) {
                return;
            }
            for (auto& point : points) {
                point.weight /= autoscore;
            }
        }
        void normalize_groups_within() {
            auto& groups = _dataset.get_groups();
            for (_progress=0; _progress<groups.size(); ++_progress) {
                normalize_group_within(groups[_progress].get_points());
            }
            get_logger().notice("Normalized groups within");
        }

        void normalize_between_groups(std::vector<Types::Point<T>>& points) {
            for (auto& point : points) {
                const T density = _density_map.get_value(point.x, point.y, point.z);
                if (density) {
                    point.weight /= density;
                } else {
                    point.weight = static_cast<T>(0.0);
                }
            }
        }
        void normalize_between_groups() {
            auto& groups = _dataset.get_groups();
            for (_progress=0; _progress<groups.size(); ++_progress) {
                normalize_between_groups(groups[_progress].get_points());
            }
            get_logger().notice("Normalized groups using density map");
        }

        // members

        const LinkRbrain::Models::Dataset<T>& _original_dataset;
        size_t _original_dataset_hash;
        LinkRbrain::Models::Dataset<T> _dataset;
        Scorer _scorer;
        Types::Array3D<T> _density_map;
        Status _status;
        size_t _progress;
        std::shared_ptr<Caching::ScorerCache<T>> _points_cache;
        std::shared_ptr<Caching::ScorerCache<T>> _groups_cache;
        std::unordered_map<const Models::Group<T>*, size_t> _groups_indexes;

    };


} // LinkRbrain::Scoring


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CORRELATOR_HPP
