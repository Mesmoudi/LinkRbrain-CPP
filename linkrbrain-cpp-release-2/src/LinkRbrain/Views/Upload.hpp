#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__UPLOAD_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__UPLOAD_HPP


#include "./BaseView.hpp"

#include "Types/DateTime.hpp"
#include "Conversion/JSON/serialize.hpp"
#include "Conversion/Base64/parse.hpp"
#include "Exceptions/GenericExceptions.hpp"

#include <filesystem>
#include <fstream>


namespace LinkRbrain::Views {

    template <typename T>
    class Upload : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef Controllers::AppController<T> AppController;

        virtual void POST(const Request& request, Response& response, AppController& app) {
            // prepare JSON output
            Types::Variant files;
            files.set_vector();
            // register files sent with a form
            for (const auto& [key, file] : request.files.get_all()) {
                // save and get computed path
                const std::filesystem::path path = save(file.filename, file.data);
                // append to output data
                files.push_back({
                    {"key", file.key},
                    {"filename", file.filename},
                    {"path", path},
                    {"size", file.data.size()}
                });
            }
            // register files sent with JSON
            if (request.data.is_map() && request.data.has("files") && request.data.get("files").is_vector()) {
                int index = 0;
                for (const auto& file : request.data.get("files").get_vector()) {
                    // fetch data
                    const std::string filename = file.get("filename").get_string();
                    const std::string format = file.get("format").get_string();
                    std::string data = file.get("data").get_string();
                    // interpret format
                    if (format == "base64") {
                        data = Conversion::Base64::straight_parse(data);
                    } else if (format != "raw") {
                        throw Exceptions::BadDataException("Bad format for file: `" + format + "`, expected `raw` or `base64`");
                    }
                    // save
                    const std::filesystem::path path = save(filename, data);
                    files.push_back({
                        {"key", std::to_string(index++)},
                        {"filename", filename},
                        {"path", path},
                        {"size", data.size()}
                    });
                }
            }
            // response depends on what has been requested
            const auto it = request.headers.find("Content-Type");
            if (it != request.headers.end() && it->second.find("multipart/form-data") == 0) {
                // redirection is expected
                const std::string url = "/upload?" + Conversion::JSON::serialize(files);
                response.headers["Location"] = url;
                response.code = 302;
            } else {
                // data response is expected
                response.data = {{"files", files}};
            }
        }

        std::filesystem::path save(const std::filesystem::path original_filename, const std::string& data) const {
            const Types::DateTime now = Types::DateTime::now();
            // make new filename
            std::string filename = std::to_string(now.get_timestamp());
            filename += '_';
            filename += original_filename.filename();
            // save file
            std::filesystem::path path = now.get_date_string();
            std::filesystem::path directory = "tmp/uploads" / path;
            std::filesystem::create_directories(directory);
            path /= filename;
            std::ofstream(directory / filename).write(data.data(), data.size());
            // return computed path
            return path;
        }

    };

} // LinkRbrain::Views


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__UPLOAD_HPP
