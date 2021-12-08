#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__DATASETCONTROLLER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__DATASETCONTROLLER_HPP


#include <set>
#include <map>
#include <filesystem>

#include "Exceptions/GenericExceptions.hpp"
#include "Logging/Loggable.hpp"

#include "LinkRbrain/Parsing/GenomicsV1Parser.hpp"
#include "LinkRbrain/Parsing/FunctionsV1Parser.hpp"
#include "LinkRbrain/Models/Dataset.hpp"
#include "LinkRbrain/Models/Query.hpp"
#include "LinkRbrain/Scoring/Correlator.hpp"

#include "Types/DateTime.hpp"


namespace LinkRbrain::Controllers {

    template <typename T>
    class DatasetController : public Logging::Loggable {
    public:

        DatasetController(const size_t organ_id, const size_t id, const std::filesystem::path& path, const std::string& label) : _path(path) {
            _path = path;
            _dataset.reset(
                new Models::Dataset<T>(organ_id, id, label)
            );
            std::filesystem::create_directories(path);
            get_logger().notice("Created dataset " + label + " with id " + std::to_string(id) + " at " + path.native());
        }
        DatasetController(const std::filesystem::path& path) {
            load(path);
        }

        // day-to-day operations

        Models::Dataset<T>& get_instance() {
            if (_dataset.get() == NULL) {
                except("No dataset instance");
            }
            return *_dataset;
        }
        const Scoring::Correlator<T>& get_correlator() const {
            if (_correlator.get() == NULL) {
                throw Exceptions::NotFoundException("Dataset " + _dataset->get_label() + " has not instanciated any correlator", {
                    {"dataset", _dataset->get_label()},
                    {"missing", "correlator"},
                });
            }
            return *_correlator;
        }
        Scoring::Correlator<T>& get_correlator() {
            if (_correlator.get() == NULL) {
                throw Exceptions::NotFoundException("Dataset " + _dataset->get_label() + " has not instanciated any correlator", {
                    {"dataset", _dataset->get_label()},
                    {"missing", "correlator"},
                });
            }
            return *_correlator;
        }

        // initialization stuff

        template <typename Parser, typename ...ParserArgsTypes>
        void parse(ParserArgsTypes... parser_args) {
            Parser::parse(*_dataset, parser_args...);
            get_logger().notice("Parsed " + _dataset->get_label() + " dataset");
            //
            std::ofstream buffer(_path / "data");
            get_logger().debug("Opened " + (_path / "data").native() + " for writing");
            Conversion::Binary::serialize(buffer, *_dataset);
            get_logger().message("Saved " + _dataset->get_label() + " dataset to " + (_path / "data").native());
        }
        void finish_correlator() {
            if (get_correlator().get_status() < Scoring::Correlator<T>::Status::CachedPoints) {
                get_correlator().compute_points_cache(Scoring::Caching::File, _path / "correlator" / "points_cache");
                get_correlator().save_config(_path / "correlator");
            }
            if (get_correlator().get_status() < Scoring::Correlator<T>::Status::CachedGroups) {
                get_correlator().compute_groups_cache(Scoring::Caching::File, _path / "correlator" / "groups_cache");
                get_correlator().save_config(_path / "correlator");
            }
        }
        void initialize_correlator(const T resolution, const Scoring::Scorer::Mode mode, const T diameter) {
            _correlator.reset(
                new Scoring::Correlator<T>(
                    *_dataset,
                    resolution,
                    mode,
                    diameter
                )
            );
            get_correlator().save(_path / "correlator");
            get_correlator().compute_points_cache(Scoring::Caching::File, _path / "correlator" / "points_cache");
            get_correlator().compute_groups_cache(Scoring::Caching::File, _path / "correlator" / "groups_cache");
            get_correlator().save_config(_path / "correlator");
        }
        const bool has_correlator() const {
            return (const bool) _correlator;
        }
        void save_correlator() {
            if (!_correlator) {
                throw Exceptions::NotFoundException("No correlator was found, cannot save", {});
            }
            get_correlator().save_config(_path / "correlator");
        }
        const typename Scoring::Correlator<T>::Status get_correlator_status() const {
            if (!_correlator) {
                return Scoring::Correlator<T>::Status::None;
            }
            return get_correlator().get_status();
        }
        const std::string get_correlator_status_name() const {
            if (!_correlator) {
                return "(none)";
            }
            return get_correlator().get_status_name();
        }


        void load_data(const std::filesystem::path& path) {
            std::ifstream file(path);
            if (!file) {
                throw Exceptions::BadDataException("DatasetController::load_data could not open " + path.native());
            }
            _dataset.reset(new Models::Dataset<T>(file));
            get_logger().notice("Loaded dataset data from file", path, "(with", _dataset->get_groups().size(), "groups)");
        }
        void load_data() {
            if (_path.empty()) {
                throw Exceptions::NotFoundException("No default path to load DatasetController data");
            }
            load_data(_path);
        }
        void load_correlator(const std::filesystem::path& path) {
            _correlator.reset(
                new LinkRbrain::Scoring::Correlator<T>(*_dataset, path)
            );
            if (std::filesystem::is_regular_file(path / "points_cache")) {
                get_correlator().load_points_cache(Scoring::Caching::File, path / "points_cache");
                get_logger().notice("Loaded dataset correlator points cache from file", path / "points_cache");
                get_correlator().load_groups_cache(Scoring::Caching::File, path / "groups_cache");
                get_logger().notice("Loaded dataset correlator groups cache from file", path / "groups_cache");
            }
            get_logger().notice("Loaded dataset correlator from file", path);
        }
        void load(const std::filesystem::path& path) {
            _path = path;
            get_logger().debug("Loading dataset data from ", path);
            load_data(path / "data");
            if (std::filesystem::is_directory(path / "correlator")) {
                get_logger().debug("Loading dataset correlator from ", path);
                load_correlator(path / "correlator");
            }
            get_logger().message("Loaded dataset controller from folder ", path);
        }

        void save_data(const std::filesystem::path& path) {
            std::ofstream file(path);
            if (!file) {
                throw Exceptions::BadDataException("DatasetController::save_data could not open " + path.native());
            }
            Conversion::Binary::serialize(file, *_dataset);
            file.flush();
            get_logger().notice("Saved dataset data to file", path, "(with", _dataset->get_groups().size(), "groups)");
        }
        void save_data() {
            if (_path.empty()) {
                throw Exceptions::NotFoundException("No default path to save DatasetController data");
            }
            save_data(_path / "data");
        }

        void save(const std::filesystem::path& path) {
            throw Exceptions::NotImplementedException("DatasetController::save has not yet been implemented");
        }
        void save() {
            if (_path.empty()) {
                throw Exceptions::NotFoundException("No default path to save DatasetController");
            }
            save(_path);
        }

        void remove() {
            std::filesystem::remove_all(_path);
            get_logger().warning("Removed dataset located at", _path);
        }

        void serialize_group(Types::Variant& destination, const LinkRbrain::Models::Group<T>& group, const bool with_points=true) const {
            destination = {
                {"id", group.get_id()},
                {"label", group.get_label()},
                {"metadata", group.get_metadata()}
            };
            if (with_points) {
                destination["points"].set_vector();
                auto& points = destination["points"].get_vector();
                for (const auto& point : group.get_points()) {
                    points.push_back({point.x, point.y, point.z, point.weight});
                }
            }
        }
        void serialize_group(Types::Variant& destination, const size_t group_id, const bool with_points=true) const {
            return serialize_group(
                destination,
                _dataset->get_group(group_id),
                with_points
            );
        }
        void serialize_groups(Types::Variant& destination, const std::vector<std::string> keywords={}, const std::unordered_set<uint64_t> identifiers={}, const size_t offset=0, const size_t limit=20, const bool with_points=false) const {
            auto& destination_data = destination["data"];
            destination_data.set_vector();
            std::vector<Types::Variant>& destination_groups = destination_data.get_vector();
            const auto& groups = _dataset->get_groups();
            Types::Variant serialized_group;
            size_t count = 0;
            // if both filter by keyword and identifier are applied
            if (keywords.size() && identifiers.size()) {
                throw Exceptions::BadDataException("Please do not filter groups by both keywords and identifiers. It does not make any sense.");
            }
            // keywords filtering
            if (keywords.size()) {
                // first pass: scored filtering
                std::multimap<float, const Models::Group<T>*> scored_groups;
                for (const auto& group : groups) {
                    std::string label = group.get_label();
                    std::transform(label.begin(), label.end(), label.begin(), tolower);
                    float score = 0.f;
                    bool has_failed = false;
                    for (const std::string& keyword : keywords) {
                        if (label.find(keyword) != std::string::npos) {
                            score += 1.00;
                        } else if (group.get_metadata().contains(keyword, true)) {
                            score += 0.25;
                        } else {
                            has_failed = true;
                            break;
                        }
                    }
                    if (!has_failed) {
                        scored_groups.insert({score, &group});
                    }
                }
                // second pass: serialization
                for (auto it=scored_groups.rbegin(); it!=scored_groups.rend(); ++it) {
                    ++count;
                    // skip inclusion if not within pagination
                    if (count <= offset) {
                        continue;
                    }
                    if (count > offset + limit) {
                        continue;
                    }
                    // serialization itself
                    serialize_group(serialized_group, *it->second, with_points);
                    destination_groups.push_back(serialized_group);
                }
            }
            // identifier filtering or no filtering
            else {
                for (const auto& group : groups) {
                    // is this id a good id?
                    if (identifiers.size() && identifiers.find(group.get_id()) == identifiers.end()) {
                        continue;
                    }
                    // if we reach here, then this is a result
                    ++count;
                    // skip inclusion if not within pagination
                    if (count <= offset) {
                        continue;
                    }
                    if (count > offset + limit) {
                        continue;
                    }
                    // serialization itself
                    serialize_group(serialized_group, group, with_points);
                    destination_groups.push_back(serialized_group);
                }
            }
            // serialize pagination
            destination["pagination"] = {
                {"offset", offset},
                {"limit", limit},
                {"count", count}};
        }
        void serialize(Types::Variant& destination, const bool with_groups=true) const {
            destination = {
                {"id", _dataset->get_id()},
                {"organ_id", _dataset->get_organ_id()},
                {"label", _dataset->get_label()},
                {"metadata", _dataset->get_metadata()}};
            if (with_groups) {
                serialize_groups(destination["groups"]);
            }
        }

        void compute(Models::Query& query, const bool with_graph=true) {
            get_logger().debug("Start computing query ", query.id, " with", with_graph?"":"out", " graph");
            query.is_computed = false;
            // parameters
            const size_t limit = query.settings.get("correlations", Types::VariantMap()).get("limit", 10);
            get_logger().detail("Fetched query groups as vector");
            // prepare query & check groups
            query.correlations.unset();
            const auto& query_groups = query.groups.get_vector();
            if (query_groups.size() == 0) {
                return;
            }
            get_logger().detail("Fetched query groups as vector");
            query.correlations.set_vector();
            std::vector<Types::Variant>& query_correlations = query.correlations.get_vector();
            get_logger().detail("Prepared correlations as vector");
            // parse points
            std::vector<std::vector<Types::Point<T>>> query_groups_points(query_groups.size());
            size_t query_group_index = 0;
            for (const auto& group : query_groups) {
                std::vector<Types::Point<T>>& points = query_groups_points[query_group_index++];
                for (const auto& _point : group["points"].get_vector()) {
                    const auto& point = _point.get_vector();
                    points.push_back({
                        point[0].get_number(),
                        point[1].get_number(),
                        point[2].get_number(),
                        point[3].get_number()});
                }
            }
            get_logger().detail("Parsed points");
            // compute correlations
            const Scoring::ScoredGroupList correlations = get_correlator().correlate(
                query_groups_points, // points
                true, // order
                limit, // limit
                false, // force_uncached
                false); // use_interpolation
            get_logger().detail("Computed correlations");
            // format correlations
            for (const auto& correlation : correlations) {
                std::vector<T> scores = correlation.scores;
                scores.insert(scores.begin(), correlation.overall_score);
                query_correlations.push_back({
                    {"id", correlation.group.get_id()},
                    {"label", correlation.group.get_label()},
                    {"metadata", correlation.group.get_metadata()},
                    {"scores", scores}});
            }
            get_logger().debug("Computed & formatted correlations");
            // graph, if asked for
            if (with_graph) {
                query.graph.clear();
                // instanciate graph
                const size_t query_groups_count = correlations.get_count();
                size_t dataset_groups_count = 0;
                if (correlations.size()) {
                    dataset_groups_count = query.settings.get("graph", Types::VariantMap()).get("limit", -1);
                    if (dataset_groups_count > correlations.size()) {
                        dataset_groups_count = correlations.size();
                    }
                }
                Graph::Graph<T> graph(dataset_groups_count + query_groups_count);
                get_logger().detail("Making graph: instanciated graph with ", 1 + dataset_groups_count, " nodes");
                // integrate links between core query group nodes
                for (size_t i = 0; i < query_groups_count; i++) {
                    for (size_t j = 0; j < i; j++) {
                        const double weight = get_correlator().score(query_groups_points[i], query_groups_points[j]);
                        graph.add_link(j, i, weight);
                    }
                }
                get_logger().detail("Making graph: added ", dataset_groups_count, " core links");
                // integrate links between core query group nodes and correlated dataset group nodes
                for (size_t i = 0; i < dataset_groups_count; i++) {
                    for (size_t j = 0; j < query_groups_count; j++) {
                        const double weight = correlations[i].scores[j];
                        graph.add_link(j, i+query_groups_count, weight);
                    }
                }
                get_logger().detail("Making graph: added ", dataset_groups_count, " core links");
                // integrate links between correlated groups nodes
                size_t n = 0;
                for (size_t i = 0; i < dataset_groups_count; i++) {
                    const auto scores = get_correlator().compute_group_scores(correlations[i].group);
                    for (size_t j = 0; j < i; j++) {
                        graph.add_link(i+query_groups_count, j+query_groups_count, scores[j]);
                        ++n;
                    }
                }
                get_logger().detail("Making graph: added ", n, " links between groups");
                // compute
                graph.compute();
                get_logger().detail("Making graph: computed layout");
                graph.normalize();
                get_logger().detail("Making graph: normalized layout");
                // format graph nodes: query groups
                query.graph["nodes"];
                size_t node_index = 0;
                for (const auto& query_group : query_groups) {
                    if (graph.nodes[node_index].has_valid_coordinates()) {
                        query.graph["nodes"].push_back({
                            {"source", "query"},
                            {"label", query_group["label"]},
                            {"hue", query_group.get("hue")},
                            {"x", graph.nodes[node_index].x},
                            {"y", graph.nodes[node_index].y},
                        });
                    }
                    ++node_index;
                }
                get_logger().detail("Formatted graph nodes: query groups");
                // format graph nodes: dataset groups
                for (size_t i = 0; i < dataset_groups_count; i++) {
                    query.graph["nodes"].push_back({
                        {"source", "dataset"},
                        {"label", correlations[i].group.get_label()},
                        {"x", graph.nodes[node_index].x},
                        {"y", graph.nodes[node_index].y},
                    });
                    ++node_index;
                }
                get_logger().detail("Formatted graph nodes: dataset groups");
                // format graph edges: initialize matrix
                auto& edges = query.graph["edges"];
                edges.set_vector();
                edges.get_vector().resize(graph.count, Types::VariantVector(graph.count, 0.0));
                get_logger().detail("Formatted graph edges: initialized matrix");
                // format graph edges: go!
                for (size_t j = 0; j < graph.count; j++) {
                    for (size_t i = 0; i < j; i++) {
                        edges[i][j] = graph.get_edge(i, j).k_spring;
                    }
                }
                get_logger().detail("Formatted graph edges: copy");
                get_logger().debug("Computed & formatted graph");
            }
            // the end!
            query.is_computed = true;
        }

    protected:

        virtual const std::string get_logger_name() {
            if (_dataset) {
                return "DatasetController[" + std::to_string(_dataset->get_id()) + "|" + _dataset->get_label() + "]";
            }
            return "DatasetController";
        }

    private:

        std::filesystem::path _path;
        std::shared_ptr<Models::Dataset<T>> _dataset;
        std::shared_ptr<LinkRbrain::Scoring::Correlator<T>> _correlator;
    };

} // LinkRbrain::Controllers


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__DATASETCONTROLLER_HPP
