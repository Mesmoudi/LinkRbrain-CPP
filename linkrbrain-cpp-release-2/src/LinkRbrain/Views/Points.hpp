#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__POINTS_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__POINTS_HPP


#include "Types/Point.hpp"
#include "LinkRbrain/Parsing/TextParser.hpp"

#include "./BaseView.hpp"

#include <istream>
#include <sstream>
#include <filesystem>
#include <vector>


namespace LinkRbrain::Views {


    template <typename T>
    class Points : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            // destination data
            auto& points = response.data["points"];
            points.set_vector();
            const std::string source_type = request.query.get("source");
            const std::string format = request.query.get("format");
            LinkRbrain::Models::Group<T> group;
            if (source_type == "upload") {
                for (const std::string& path : request.query.get_all("files")) {
                    std::filesystem::path source_path = "tmp/uploads";
                    source_path /= path;
                    if (format == "text") {
                        std::ifstream source(source_path);
                        Parsing::TextParser().parse(group, source);
                    } else if (format == "nifti") {
                        const double resolution = std::stof(request.query.get("resolution", "6"));
                        LinkRbrain::Parsing::NiftiParser().parse(group, source_path, resolution);
                    } else {
                        throw Exceptions::BadDataException("Unknown format: " + format);
                    }
                }
            } else if (source_type == "data") {
                const std::string source_data = request.query.get("data", "");
                if (format == "text") {
                    std::stringstream source(source_data);
                    Parsing::TextParser().parse(group, source);
                } else {
                    throw Exceptions::BadDataException("Only 'text' format is supported for 'data' source type, but this was found instead: " + format);
                }
            } else {
                throw Exceptions::BadDataException("Unknown source type: " + source_type);
            }
            // add parsed points to output
            for (const auto& point : group.get_points()) {
                points.push_back(point.values);
            }
        }

    };


} // LinkRbrain::Views


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__POINTS_HPP
