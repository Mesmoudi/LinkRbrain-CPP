#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__FUNCTIONSV1PARSER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__FUNCTIONSV1PARSER_HPP


#include "./Parser.hpp"

#include <stdio.h>

#include <filesystem>
#include <fstream>
#include <set>


namespace LinkRbrain::Parsing {

    class FunctionsV1Parser : public Parser {
    public:

        template <typename T>
        static void parse(LinkRbrain::Models::Dataset<T>& dataset, const std::string& path, const std::string& prefix="/", const std::string& suffix=".") {
            // quick check
            if (!std::filesystem::is_directory(path)) {
                throw Exceptions::NotFoundException("Not a directory: " + path, {});
            }
            // sort directory entries
            std::set<std::string> paths;
            for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    paths.insert(entry.path());
                }
            }
            // parse corresponding files
            for (const std::string& path : paths) {
                size_t start_position = prefix.size() ? (path.rfind(prefix) + prefix.size()) : 0;
                size_t end_position = suffix.size() ? path.rfind(suffix) : path.size();
                std::string label = path.substr(
                    start_position,
                    end_position - start_position
                );
                LinkRbrain::Models::Group<T>& group = dataset.add_group(label);
                std::ifstream f(path);
                if (f.is_open()) {
                    double x, y, z, weight;
                    std::string line;
                    while (std::getline(f, line)) {
                        int n = sscanf(line.c_str(), "%lf %lf %lf %lf", &x, &y, &z, &weight);
                        if (n == 3) {
                            group.add_point(x, y, z, 1.0);
                        } else if (n == 4) {
                            group.add_point(x, y, z, weight);
                        } else {
                            continue;
                        }
                    }
                }
            }
        }

    };


} // LinkRbrain::Parsing


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__FUNCTIONSV1PARSER_HPP
