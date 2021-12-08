#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__DATASET_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__DATASET_HPP


#include <csignal>

#include "LinkRbrain/Controllers/DataController.hpp"
#include "LinkRbrain/Parsing/TextParser.hpp"
#include "LinkRbrain/Parsing/NiftiParser.hpp"
#include "CLI/Arguments/Command.hpp"
#include "CLI/Display/Canvas.hpp"

#include "./linkrbrain.hpp"
#include "./linkrbrain_organ.hpp"


namespace LinkRbrain::Commands {

    void dataset(const CLI::Arguments::CommandResult& options) {
    }


    void _show_datasets(Types::Table& table, const LinkRbrain::Controllers::OrganController<T>& organ_controller) {
        for (const auto& dataset_controller : organ_controller.get_datasets()) {
            table.add_row({
                std::to_string(organ_controller.get_instance().get_id()),
                organ_controller.get_instance().get_label(),
                std::to_string(dataset_controller->get_instance().get_id()),
                dataset_controller->get_instance().get_label(),
                std::to_string(dataset_controller->get_correlator_status_name()),
                std::to_string(dataset_controller->get_instance().get_groups().size())
            });
        }
    }

    void dataset_extrema(const CLI::Arguments::CommandResult& options) {
        // retrieve organ & dataset controller
        auto& organ_controller = _get_organ_controller(options.get("organ"));
        auto& dataset_controller = _get_dataset_controller(organ_controller, options.get("dataset"));
        // compute extrema
        const auto extrema = dataset_controller.get_instance().compute_extrema();
        // show extrema
        std::cout << "x : " << extrema.min.x << " ... " << extrema.max.x << '\n';
        std::cout << "y : " << extrema.min.y << " ... " << extrema.max.y << '\n';
        std::cout << "z : " << extrema.min.z << " ... " << extrema.max.z << '\n';
        std::cout << "weight : " << extrema.min.weight << " ... " << extrema.max.weight << '\n';
    }

    void dataset_list(const CLI::Arguments::CommandResult& options) {
        Types::Table table("Organ id", "Organ label", "Dataset id", "Dataset label", "Correlator status", "Groups");
        size_t count = 0;
        if (options.has("organ")) {
            auto& organ_controller = _get_organ_controller(options.get("organ"));
            _show_datasets(table, organ_controller);
            std::cout << table.get_rows().size() << " dataset(s) are registered for organ '" << organ_controller.get_instance().get_label() << "'";
        } else {
            for (const auto& organ_controller : _get_data_controller().get_organs()) {
                _show_datasets(table, *organ_controller);
            }
            std::cout << table.get_rows().size() << " dataset(s) are registered";
        }
        std::cout << " in '" << data_path << "':\n" << table;
    }

    void dataset_list_groups(const CLI::Arguments::CommandResult& options) {
        const bool with_metadata = options.get("with-metadata") == "true";
        const bool with_points = options.get("with-points") == "true";
        // retrieve organ & dataset controller
        auto& organ_controller = _get_organ_controller(options.get("organ"));
        auto& dataset_controller = _get_dataset_controller(organ_controller, options.get("dataset"));
        // fill table
        std::vector<std::string> table_header = {"Group id", "Group label", "Points count"};
        Types::Table table(table_header);
        for (const auto& group : dataset_controller.get_instance().get_groups()) {
            std::vector<std::string> row = {
                std::to_string(group.get_id()),
                group.get_label(),
                std::to_string(group.get_points().size())
            };
            if (with_metadata) {
                row.push_back(Conversion::JSON::serialize(group.get_metadata()));
            }
            table.add_row(row);
        }
        // show table
        if (options.get("format") == "table") {
            std::cout << table;
        } else if (options.get("format") == "csv") {
            // show table header
            bool is_first = true;
            for (const auto& cell : table.get_header()) {
                if (is_first) is_first = false;
                else std::cout << ";";
                std::cout << cell;
            }
            std::cout << '\n';
            // show table contents
            for (const auto& row : table.get_rows()) {
                bool is_first = true;
                for (const auto& cell : row) {
                    if (is_first) is_first = false;
                    else std::cout << ";";
                    std::cout << cell;
                }
                std::cout << '\n';
            }
        } else {
            std::cerr << "Unrecognized format for output: " << options.get("format") << '\n';
        }
    }

    void dataset_add(const CLI::Arguments::CommandResult& options) {
        // extract parameters
        const std::string dataset_label = options.get("label");
        const std::string source_path = options.get("source");
        const std::string source_type = options.get("type");
        const T resolution = std::stod(options.get("resolution"));
        const T radius = std::stod(options.get("radius"));
        LinkRbrain::Scoring::Scorer::Mode scoring_mode;
        if (options.get("scoring-mode") == "spheres") {
            scoring_mode = LinkRbrain::Scoring::Scorer::Sphere;
        } else if (options.get("scoring-mode") == "distance") {
            scoring_mode = LinkRbrain::Scoring::Scorer::Distance;
        } else {
            throw Exceptions::BadDataException("Unrecognized scoring mode: " + options.get("scoring-mode"), {});
        }
        // create or retrieve dataset
        auto& organ_controller = _get_organ_controller(options.get("organ"));
        static LinkRbrain::Controllers::DatasetController<T>* dataset_controller;
        bool creation = false;
        try {
            dataset_controller = & organ_controller.get_dataset(options.get("label"));
        } catch (const Exceptions::NotFoundException&) {
            dataset_controller = & _get_data_controller().add_dataset(organ_controller, options.get("label"));
            creation = true;
        }
        auto& dataset = dataset_controller->get_instance();
        std::cout << (creation ? "Created" : "Loaded") << " dataset with label '" << dataset.get_label() << "' and identifier " << dataset.get_id() << '\n';
        // parse data into dataset when necessary; on failure, remove dataset
        if (creation) {
            // control source type
            if (source_type != "allen" && source_type != "barycenters") {
               throw Exceptions::BadDataException("Unrecognized value '" + source_type + "' for source type", {});
            }
            // try to parse
            try {
                if (source_type == "barycenters") {
                    dataset_controller->parse<LinkRbrain::Parsing::FunctionsV1Parser>(
                        source_path,
                        options.get("prefix"),
                        options.get("suffix")
                    );
                } else if (source_type == "allen") {
                    dataset_controller->parse<LinkRbrain::Parsing::GenomicsV1Parser>(
                        source_path
                    );
                }
            }
            // if parsing failed, remove dataset and exit
            catch (const std::exception& error) {
                std::cout << "Error while parsing dataset: " << error.what() << '\n';
                organ_controller.remove_dataset(*dataset_controller);
                std::cout << "Deleted dataset to avoid data corruption.\n";
                exit(1);
            }
            auto& groups = dataset.get_groups();
            std::cout << "Integrated " << groups.size() << " groups into dataset";
            if (groups.size()) {
                std::cout << ", from '" << groups.begin()->get_label() << "' to '" << groups.rbegin()->get_label() << "'";
            }
            std::cout << '\n';
        }
        // initialize correlator (involves caching, this step can be quite lengthy)
        const auto correlator_status = dataset_controller->get_correlator_status();
        if (!dataset_controller->has_correlator() || correlator_status < Scoring::Correlator<T>::Status::CachedGroups) {
            // setup signal handler
            static const std::vector<int> handled_signals = {SIGTERM, SIGINT, SIGABRT};
            for (const int handled_signal : handled_signals) {
                std::signal(handled_signal, [] (int signum) {
                    std::cout << "\nProgram got interrupted!\n";
                    std::cout << "Saving correlator...\n";
                    dataset_controller->save_correlator();
                    std::cout << "Saved correlator.\n\n";
                    exit(0);
                });
            }
            // compute correlator cache
            if (dataset_controller->has_correlator()) {
                dataset_controller->finish_correlator();
            } else {
                dataset_controller->initialize_correlator(resolution, scoring_mode, radius);
            }
            std::cout << "\nComputed correlator" << '\n';
        }
    }

    void dataset_remove(const CLI::Arguments::CommandResult& options) {
        // retrieve organ & dataset controller
        auto& organ_controller = _get_organ_controller(options.get("organ"));
        auto& dataset_controller = _get_dataset_controller(organ_controller, options.get("dataset"));
        // initialize final message
        std::stringstream message;
        message << "Deleted dataset '" << dataset_controller.get_instance().get_label() << "' with identifier " << dataset_controller.get_instance().get_id();
        message << " from organ '" << organ_controller.get_instance().get_label() << "' with identifier " << organ_controller.get_instance().get_id() << '\n';
        // delete
        organ_controller.remove_dataset(dataset_controller);
        // the end!
        std::cout << message.str();
    }
    void dataset_rename(const CLI::Arguments::CommandResult& options) {
        // retrieve organ & dataset controller
        auto& organ_controller = _get_organ_controller(options.get("organ"));
        auto& dataset_controller = _get_dataset_controller(organ_controller, options.get("dataset"));
        const std::string new_name = options.get("name");
        // rename dataset
        std::stringstream message;
        std::cout << "Renaming dataset '" << dataset_controller.get_instance().get_label() << "' with identifier " << dataset_controller.get_instance().get_id();
        std::cout << " from organ '" << organ_controller.get_instance().get_label() << "' with identifier " << organ_controller.get_instance().get_id() << "\n...";
        dataset_controller.get_instance().set_label(new_name);
        dataset_controller.save_data();
        std::cout << "New name is '" << new_name << "'.\n";
    }

    const std::vector<Types::Point<T>> _get_input_points(LinkRbrain::Controllers::OrganController<T>& organ_controller, const CLI::Arguments::CommandResult& options) {
        const std::string source_type = options.get("source-type");
        // list of points
        if (source_type == "points") {
            std::vector<Types::Point<T>> points;
            for (const std::string point_as_string : options.get_all("source-point")) {
                bool valid = false;
                double x, y, z, weight;
                int n = sscanf(point_as_string.c_str(), "%lf,%lf,%lf,%lf", &x, &y, &z, &weight);
                switch (n) {
                    case 3:
                        weight = 1.0;
                    case 4:
                        valid = true;
                        break;
                    default:
                        std::cout << "Only " << n << " floating-point coordinates were provided for point, expected either 3 or 4: " << point_as_string << '\n';;
                }
                if (valid) {
                    points.push_back({x, y, z, weight});
                }
            }
            return points;
        }
        // existings group in a dataset
        else if (source_type == "group") {
            auto& dataset_controller = _get_dataset_controller(organ_controller, options.has("source-dataset") ? options.get("source-dataset") : options.get("dataset"));
            auto& dataset = dataset_controller.get_instance();
            auto& group = dataset.get_group(options.get("source-group"), options.has("source-exact"));
            std::cout << "Found group '" << group.get_label() << "' with " << group.get_points().size() << " points" << '\n';
            return group.get_points();
        }
        // text file
        else if (source_type == "text") {
            std::ifstream source(options.get("source-text"));
            LinkRbrain::Models::Group<T> group;
            LinkRbrain::Parsing::TextParser().parse(group, source);
            return group.get_points();
        }
        // nifti file
        else if (source_type == "nifti") {
            const double resolution = std::stof(options.get("resolution"));
            const std::filesystem::path path(options.get("source-nifti"));
            LinkRbrain::Models::Group<T> group;
            LinkRbrain::Parsing::NiftiParser().parse(group, path, resolution);
            return group.get_points();
        }
        // this source type has not (yet?) been implemented
        else {
            throw Exceptions::BadDataException("Unrecognized source type: " + source_type, {});
        }
    }

    void dataset_query(const CLI::Arguments::CommandResult& options) {
        // retrieve organ & dataset controller
        auto& organ_controller = _get_organ_controller(options.get("organ"));
        auto& dataset_controller = _get_dataset_controller(organ_controller, options.get("dataset"));
        // make query
        Models::Query query;
        query.settings["correlations"]["limit"] = std::stoi(options.get("limit"));
        query.groups.push_back({{"label", "Group 0"}});
        // make query group
        auto& query_group_points = query.groups[0]["points"];
        query_group_points.set_vector();
        for (const auto& point : _get_input_points(organ_controller, options)) {
            query_group_points.push_back(point.values);
        }
        // compute query correlations (and graph if requested)
        dataset_controller.compute(query, options.has("with-graph"));
        // show correlations results
        if (options.get("format") == "json") {
            Conversion::JSON::serialize(std::cout, query.correlations);
            std::cout << '\n';
        } else if (options.get("format") == "csv") {
            for (auto& item : query.correlations.get_vector()) {
                std::cout << item["scores"][0] << ';' << item["label"] << '\n';
            }
        } else if (options.get("format") == "table") {
            Types::Table table("Rank", "Score", "Group label");
            for (size_t i = 0; i < query.correlations.size(); i++) {
                table.add_row(
                    i + 1,
                    query.correlations[i]["scores"][0].get_floating(),
                    query.correlations[i]["label"].get_string()
                );
            }
            std::cout << "Found " << query.correlations.size() << " correlation results with dataset '" << dataset_controller.get_instance().get_label() << "':\n";
            std::cout << table;
        } else if (options.get("format") == "text") {
            std::cout << "Found " << query.correlations.size() << " correlation results with dataset '" << dataset_controller.get_instance().get_label() << "':\n";
            for (auto& item : query.correlations.get_vector()) {
                std::cout << std::left << std::setw(15) << item["scores"][0] << ' ' << item["label"] << '\n';
            }
        } else {
            std::cout << "Invalid value for option 'format': " << options.get("format") << '\n';
        }
        // compute & show graph if required
        if (options.has("with-graph")) {
            std::cout << "Computed graph:" << '\n';
            if (options.get("with-graph") == "table") {
                // std::cout << graph << '\n';
            } else if (options.get("with-graph") == "json") {
                std::cout << Conversion::JSON::serialize(query.graph) << '\n';
            } else {
                CLI::Display::Canvas canvas;
                canvas.set_mode(CLI::Display::Canvas::Mode::Fit);
                const auto& graph_nodes = query.graph["nodes"];
                for (size_t i = 0; i < graph_nodes.size(); i++) {
                    const auto& node = graph_nodes[i];
                    canvas.write(node["x"], node["y"], node["label"]);
                }
                std::cout << canvas << '\n';
            }
        }
    }

} // LinkRbrain::Commands


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__DATASET_HPP
