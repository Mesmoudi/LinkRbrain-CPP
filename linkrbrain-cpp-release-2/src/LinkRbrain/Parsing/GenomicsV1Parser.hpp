#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__GENOMICSV1PARSER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__GENOMICSV1PARSER_HPP


#include <string>
#include <vector>
#include <unordered_map>

// #include "Types/Variant.hpp"
#include "LinkRbrain/Models/Dataset.hpp"
#include "Reading/CSVReader.hpp"
#include "Logging/Loggers.hpp"


namespace LinkRbrain::Parsing {


    template <typename T>
    inline const T convert(const std::string& source) {
        except("Unsupported type");
    }
    template <>
    inline const float convert<float>(const std::string& source) {
        return std::stof(source);
    }
    template <>
    inline const double convert<double>(const std::string& source) {
        return std::stod(source);
    }
    template <>
    inline const long double convert<long double>(const std::string& source) {
        return std::stold(source);
    }
    template <>
    inline const int convert<int>(const std::string& source) {
        return std::stoi(source);
    }
    template <>
    inline const long convert<long>(const std::string& source) {
        return std::stol(source);
    }
    template <>
    inline const long long convert<long long>(const std::string& source) {
        return std::stoll(source);
    }
    template <>
    inline const unsigned long convert<unsigned long>(const std::string& source) {
        return std::stoul(source);
    }
    template <>
    inline const unsigned long long convert<unsigned long long>(const std::string& source) {
        return std::stoull(source);
    }


    class GenomicsV1Parser {
    public:

        template <typename T>
        static void parse_probes(LinkRbrain::Models::Dataset<T>& dataset, const std::filesystem::path& path) {
            Reading::CSVReader csv(path, 1);
            std::vector<std::string> columns;
            //
            std::unordered_map<uint64_t, Types::Variant> by_gene_id;
            while (csv.parse_line(columns)) {
                // eliminate non-refseq
                if (columns[6].size() == 0) {
                    continue;
                }
                // one gene can have many probes
                const uint64_t gene_id = convert<uint64_t>(columns[2]);
                Types::Variant& metadata = by_gene_id[gene_id];
                if (metadata.get_type() != Types::Variant::Map) {
                    metadata = {
                        {"gene_id", gene_id},
                        {"gene_symbol", columns[3]},
                        {"gene_name", columns[4]},
                        {"entrez_id", convert<uint64_t>(columns[5])},
                        {"chromosome", columns[6]},
                        {"probes", Types::VariantVector()}
                    };
                }
                Types::Variant& probes = metadata["probes"];
                probes.push_back({
                    {"probe_id", convert<uint64_t>(columns[0])},
                    {"probe_name", columns[1]}
                });
            }
            // now, let's build the corresponding groups
            for (const auto& [gene_id, metadata] : by_gene_id) {
                LinkRbrain::Models::Group<T>& group = dataset.add_group(metadata["gene_symbol"].get<std::string>());
                group.set_metadata(metadata);
            }
        }

        template <typename T>
        static std::vector<Types::Point<T>> parse_structures(const std::filesystem::path& path) {
            std::vector<Types::Point<T>> result;
            Reading::CSVReader csv(path, 1);
            std::vector<std::string> columns;
            while (csv.parse_line(columns)) {
                if (columns.size() < 13) {
                    continue;
                }
                result.push_back({
                    convert<T>(columns[10]),
                    convert<T>(columns[11]),
                    convert<T>(columns[12])
                });
            }
            return result;
        }

        template <typename T>
        static std::unordered_map<uint64_t, std::vector<T>> parse_expressions(const std::filesystem::path& path) {
            std::unordered_map<uint64_t, std::vector<T>> all_values;
            Reading::CSVReader csv(path, 1);
            std::vector<std::string> columns;
            while (csv.parse_line(columns)) {
                const uint64_t probe_id = convert<uint64_t>(columns[0]);
                // extract all values for this line
                std::vector<T> values;
                values.resize(columns.size() - 1);
                T value_min = +INFINITY;
                T value_max = -INFINITY;
                T value_average = 0.0;
                for (size_t i=1, n=columns.size(); i<n; ++i) {
                    const T value = convert<T>(columns[i]);
                    values[i - 1] = value;
                    if (value < value_min) value_min = value;
                    if (value > value_max) value_max = value;
                    value_average += value;
                }
                // normalize values
                value_average /= (T) columns.size();
                for (T& value : values) {
                    value -= value_min;
                    value /= value_average - value_min;
                }
                // store
                all_values[probe_id] = values;
            }
            return all_values;
        }

        template <typename T>
        static void parse(LinkRbrain::Models::Dataset<T>& dataset, const std::filesystem::path& path) {
            // parse data from files
            parse_probes(dataset, path / "Probes.csv");
            get_logger().debug("Found", dataset.get_groups().size(), "probes");
            const auto structures = parse_structures<T>(path / "SampleAnnot.csv");
            get_logger().debug("Found", structures.size(), "structures");
            const auto values = parse_expressions<T>(path / "MicroarrayExpression.csv");
            get_logger().debug("Found", values.size(), "values");
            // manage points
            size_t i = 0;
            for (auto& group : dataset.get_groups()) {
                auto& probes = group.get_metadata("probes").get_vector();
                for (const auto& probe_metadata : probes) {
                    uint64_t probe_id = probe_metadata["probe_id"].template get<int64_t>();
                    std::vector<Types::Point<T>> points = structures;
                    const auto expressions_it = values.find(probe_id);
                    if (values.find(probe_id) == values.end()) {
                        continue;
                    }
                    const std::vector<T>& expressions = expressions_it->second;
                    for (int i=0, n=points.size(); i<n; ++i) {
                        points[i].weight = expressions[i];
                    }
                    group.integrate_points(points);
                }
                truncate(group.get_points(), 0.17);
                std::cout << "Group " << ++i << "/" << dataset.get_groups().size() << ", " << probes.size() << " probes, " << group.get_points().size() << " points                \r";
            }
            get_logger().debug("Integrated probes into groups");
        }

        static Logging::Logger& get_logger() {
            static Logging::Logger& logger = Logging::get_logger("GenomicsV1Parser");
            return logger;
        }

    private:

        template <typename T>
        static void truncate(std::vector<Types::Point<T>>& points, const T threshold) {
            // first, let's sort this mess
            std::multimap<T, Types::Point<T>> sorted_points;
            T full_sum = 0.0;
            for (const Types::Point<T>& point : points) {
                sorted_points.insert({point.weight, point});
                full_sum += point.weight;
            }
            // now, only keep what's relevant to threshold
            std::vector<Types::Point<T>> truncated_points;
            T partial_sum = 0.0;
            T partial_sum_threshold = threshold * full_sum;
            for (auto it=sorted_points.rbegin(); it!=sorted_points.rend(); ++it) {
                const Types::Point<T>& point = it->second;
                truncated_points.push_back(point);
                partial_sum += point.weight;
                if (partial_sum > partial_sum_threshold) {
                    points = truncated_points;
                    return;
                }
            }
        }

    };


} // Parsing


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__GENOMICSV1PARSER_HPP
