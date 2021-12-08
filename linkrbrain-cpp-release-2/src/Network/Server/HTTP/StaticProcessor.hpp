#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__STATICPROCESSOR_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__STATICPROCESSOR_HPP


#include "./Processor.hpp"

#include "Compression/Zlib/OStream.hpp"

#include <map>
#include <vector>
#include <fstream>
#include <filesystem>


namespace Network::Server::HTTP {


    struct StaticProcessor : public Processor {
    public:

        StaticProcessor() :
            _client_caching(true),
            _server_caching(true),
            _gzip(true) {}

        void add_path(const std::string& path) {
            _folders.push_back(path);
        }

        const std::string compute_etag(const Connection& connection) const {
            for (const auto& folder : _folders) {
                if (_client_caching) {
                    std::string fullpath = folder + '/' + connection.request.url;
                    const uint64_t last_write_time =
                        std::filesystem::last_write_time(fullpath).time_since_epoch().count();
                    std::stringstream etag_stream;
                    etag_stream << std::hex << last_write_time;
                    return "\"" + etag_stream.str() + "\"";
                }
            }
            return "";
        }

        const bool process_nocache(Connection& connection) {
            for (const auto& folder : _folders) {
                // open file
                std::string fullpath = folder + '/' + connection.request.url;
                if (std::filesystem::is_directory(fullpath)) {
                    if (*fullpath.rbegin() != '/') {
                        fullpath += "/";
                    }
                    fullpath += "index.html";
                }
                std::ifstream file(fullpath);
                // is it a good file?
                if (!file.good()) {
                    continue;
                }
                // compute ETAG
                if (_client_caching) {
                    const uint64_t last_write_time =
                        std::filesystem::last_write_time(fullpath).time_since_epoch().count();
                    std::stringstream etag_stream;
                    etag_stream << std::hex << last_write_time;
                    std::string etag = "\"" + etag_stream.str() + "\"";
                    connection.response.headers["ETag"] = etag;
                }
                // find content type
                const std::size_t last_dot = fullpath.rfind('.');
                if (last_dot != std::string::npos) {
                    const std::string ending = fullpath.substr(last_dot + 1);
                    auto it = _content_types.find(ending);
                    if (it != _content_types.end()) {
                        connection.response.headers["Content-Type"] = it->second;
                    }
                }
                // find size
                file.seekg(0, std::ios::end);
                std::size_t length = file.tellg();
                file.seekg(0, std::ios::beg);
                // read file
                if (_gzip && connection.request.headers["Accept-Encoding"].find("gzip") != std::string::npos) {
                    connection.response.headers["Content-Encoding"] = "gzip";
                    Compression::Zlib::OStream compressed(connection.response.raw);
                    std::copy(
                        std::istreambuf_iterator<char>(file),
                        std::istreambuf_iterator<char>(),
                        std::ostreambuf_iterator<char>(compressed));
                } else {
                    std::copy(
                        std::istreambuf_iterator<char>(file),
                        std::istreambuf_iterator<char>(),
                        std::ostreambuf_iterator<char>(connection.response.raw));
                }
                // that was it!
                return true;
            }
            return false;
        }
        virtual const bool process(Connection& connection) {
            // with server cache
            if (_server_caching) {
                // retrieve cache element
                const auto cache_it = _cache.find(connection.request.url);
                if (cache_it != _cache.end()) {
                    // find out if client caching can be applied
                    if (_client_caching) {
                        const auto headers_it = connection.request.headers.find("If-None-Match");
                        if (headers_it != connection.request.headers.end()) {
                            if (headers_it->second == cache_it->second.headers["ETag"]) {
                                connection.response.code = 304;
                                return true;
                            }
                        }
                    }
                    // otherwise, send full response
                    connection.response = cache_it->second;
                    return true;
                }
                // if nothing is present in server cache, generate it
                if (process_nocache(connection)) {
                    _cache.insert({connection.request.url, connection.response});
                    return true;
                }
                return false;
            }
            // without server cache
            if (_client_caching) {
                const auto it = connection.request.headers.find("If-None-Match");
                if (it != connection.request.headers.end()) {
                    if (compute_etag(connection) == it->second) {
                        connection.response.code = 304;
                        return true;
                    }
                }
            }
            return process_nocache(connection);
        }

        void reset_cache() {
            _cache.clear();
            _gzip_cache.clear();
        }

        void set_server_caching(const bool server_caching) {
            _server_caching = server_caching;
        }
        void set_client_caching(const bool client_caching) {
            _client_caching = client_caching;
        }

    private:

        std::vector<std::string> _folders;
        bool _server_caching;
        bool _client_caching;
        bool _gzip;
        std::map<std::string, Response> _cache;
        std::map<std::string, Response> _gzip_cache;
        static std::map<std::string, std::string> _content_types;
    };

    std::map<std::string, std::string> StaticProcessor::_content_types = {
        {"gif", "image/gif"},
        {"htm", "text/html"},
        {"html", "text/html"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"js", "text/javascript"},
        {"json", "application/json"},
        {"png", "image/png"},
        {"txt", "text/plain"},
        {"xml", "text/xml"},
        {"obj", "application/object"},
    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__STATICPROCESSOR_HPP
