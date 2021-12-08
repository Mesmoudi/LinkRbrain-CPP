#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__SERVER_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__SERVER_HPP


#include <arpa/inet.h>
#include <microhttpd.h>

#include <exception>
#include <vector>
#include <map>

#include "./DynamicProcessor.hpp"
#include "./StaticProcessor.hpp"
#include "./RoutingProcessor.hpp"
#include "./Connection.hpp"

#include "Exceptions/Exception.hpp"
#include "Logging/Loggable.hpp"


namespace Network::Server::HTTP {


    class Server : public Logging::Loggable {
    public:

        Server() :
            _daemon(NULL),
            _port(80),
            _threading(1),
            _timeout(30),
            _post_buffer_size(512),
            _debug(true),
            _routing(true),
            _static_processing(true),
            _dynamic_processing(true),
            _persistent(false) {}
        ~Server() {
            stop();
        }

        #ifndef CREATE_GETTER_SETTER
        #define CREATE_GETTER_SETTER(TYPE, NAME) \
                const TYPE& get_##NAME() { return _##NAME; } \
                void set_##NAME(const TYPE& NAME) { _##NAME = NAME; }
            CREATE_GETTER_SETTER(uint16_t, port)
            CREATE_GETTER_SETTER(unsigned int, threading)
            CREATE_GETTER_SETTER(unsigned int, timeout)
            CREATE_GETTER_SETTER(bool, debug)
            CREATE_GETTER_SETTER(bool, persistent)
            CREATE_GETTER_SETTER(bool, routing)
            CREATE_GETTER_SETTER(bool, static_processing)
            CREATE_GETTER_SETTER(bool, dynamic_processing)
        #endif // CREATE_GETTER_SETTER
        void set_server_caching(const bool server_caching) {
            _static_processor.set_server_caching(server_caching);
        }
        void set_client_caching(const bool client_caching) {
            _static_processor.set_client_caching(client_caching);
        }

        void start() {
            // daemon options
            _daemon_options.clear();
            _daemon_options.push_back({MHD_OPTION_CONNECTION_TIMEOUT, _timeout, NULL});
            if (_threading > 1) {
                _daemon_options.push_back({MHD_OPTION_THREAD_POOL_SIZE, _threading, NULL});
            }
            _daemon_options.push_back({MHD_OPTION_END, 0, NULL});
            // start daemon
            _daemon = MHD_start_daemon(
                // MHD_USE_SELECT_INTERNALLY, // flags
                MHD_USE_SELECT_INTERNALLY | MHD_USE_EPOLL_LINUX_ONLY | MHD_USE_EPOLL_TURBO | MHD_USE_DEBUG,
                _port, // TCP port
                NULL, NULL, // determine which clients can connect (here, everyone)
                &connection_callback, // handler called for all requests
                this, // callback parameter for all requests
                MHD_OPTION_ARRAY, _daemon_options.data(), MHD_OPTION_END // options
            );
            // what if it failed?
            if (_daemon == NULL) {
                except("Could not start web server: " + std::string(strerror(errno)));
            }
        }
        void stop() {
            if (_daemon != NULL) {
                MHD_stop_daemon(_daemon);
                _daemon = NULL;
            }
        }
        void restart() {
            if (_daemon) {
                stop();
                start();
            }
        }

        template <typename SpecificResource>
        void add_resource(const std::string& url_pattern) {
            _dynamic_processor.template add_resource<SpecificResource>(url_pattern);
        }
        void add_static_path(const std::string& path) {
            _static_processor.add_path(path);
        }
        void add_redirection(
            const std::string pattern,
            const std::string replacement,
            Redirection::Type type = Redirection::Temporary,
            const bool is_last = false)
        {
            _routing_processor.add_redirection(pattern, replacement, type, is_last);
        }

        template <typename T>
        void set_resource_parameter(T& parameter) {
            _dynamic_processor.set_resource_parameter(parameter);
        }

        static int headers_callback(void *_headers, MHD_ValueKind kind, const char* key, const char* value) {
            std::unordered_map<std::string, std::string>& headers = * (std::unordered_map<std::string, std::string>*) _headers;
            headers.insert({key, value});
            return MHD_YES;
        }
        static int query_callback(void *_query, MHD_ValueKind kind, const char* key, const char* value) {
            Types::MiniMap<std::string, std::string>& query = * (Types::MiniMap<std::string, std::string>*) _query;
            if (key && value) {
                query.insert(key, value);
            }
            return MHD_YES;
        }
        static int files_callback(void *_files, MHD_ValueKind kind, const char* key, const char* filename, const char* content_type, const char* transfer_encoding, const char* data, const uint64_t offset, const size_t size) {
            Types::MiniMap<std::string, File>& files = * (Types::MiniMap<std::string, File>*) _files;
            File* file_;
            try {
                file_ = & files.get(key);
            } catch (const Exceptions::NotFoundException&) {
                const std::pair<std::string, File> key_value = {key, {key, filename, content_type, transfer_encoding}};
                files.insert(key_value);
                file_ = & files.get(key);
            }
            if (size) {
                file_->data.append(data, size);
            }
            return MHD_YES;
        }
        static int connection_callback(
            void *_server,
            struct MHD_Connection* mhd_connection,
            const char *url,
            const char *method, const char *version,
            const char *upload_data,
            size_t *upload_size, void **con_cls)
        {
            Server& server = *(Server*) _server;
            // initialize connection when necessary
            Connection* connection_ = static_cast<Connection*>(*con_cls);
            const bool new_connection = (connection_ == NULL);
            if (new_connection) {
                *con_cls = connection_ = new Connection(
                    mhd_connection,
                    method, url);
            }
            Connection& connection = *connection_;
            // extract request information
            bool has_failed = false;
            int step;
            try {
                // extract headers & query parameters
                step = 0x10;
                MHD_get_connection_values(mhd_connection, MHD_HEADER_KIND, headers_callback, &(connection.request.headers));
                step = 0x20;
                MHD_get_connection_values(mhd_connection, MHD_GET_ARGUMENT_KIND, query_callback, &(connection.request.query));
                // extract upload data for POST, PUT, PATCH requests
                step = 0x00;
                if (method[0] == 'P') {
                    // files form
                    if (connection.request.method == "POST" && connection.request.headers["Content-Type"].find("multipart/form-data") == 0) {
                        if (new_connection) {
                            // with 65536 = _post_buffer_size
                            MHD_PostProcessor* post_processor = MHD_create_post_processor(mhd_connection, 65536, files_callback, &(connection.request.files));
                            connection.set_post_processor(post_processor);
                            return MHD_YES;
                        } else if (*upload_size) {
                            MHD_PostProcessor* post_processor = connection.get_post_processor();
                            MHD_post_process(post_processor, upload_data, *upload_size);
                            *upload_size = 0;
                            return MHD_YES;
                        } else {
                            MHD_PostProcessor* post_processor = connection.get_post_processor();
                            MHD_destroy_post_processor(post_processor);
                        }
                    // other kinds of upload
                    } else if (!connection.request.is_uploading) {
                        connection.request.is_uploading = true;
                        return MHD_YES;
                    } else if (*upload_size) {
                        connection.request.raw.write(upload_data, *upload_size);
                        *upload_size = 0;
                        return MHD_YES;
                    }
                }
                connection.request.is_uploading = false;
            } catch (std::exception& error) {
                std::string message = "Error while processing HTTP: ";
                switch (step) {
                    case 0x00:
                        message += "upload";
                        break;
                    case 0x10:
                        message += "headers";
                        break;
                    case 0x20:
                        message += "query parameters";
                        break;
                    default:
                        message += "unknown";
                        break;
                }
                connection.response.code = 400;
                connection.response.data = {
                    {"message", message},
                    {"details", error.what()}
                };
                has_failed = true;
            }
            // extract IP
            try {
                sockaddr* socket_address = MHD_get_connection_info(mhd_connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
                char ip[256];
                switch(socket_address->sa_family) {
                    case AF_INET:
                    inet_ntop(AF_INET, &(((struct sockaddr_in *)socket_address)->sin_addr), ip, sizeof(ip));
                    break;
                    case AF_INET6:
                    inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)socket_address)->sin6_addr), ip, sizeof(ip));
                    break;
                    default:
                    ip[0] = 0;
                }
                connection.request.ip = ip;
            } catch (std::exception& error) {
                server.get_logger().warning(
                    "Error while computing IP address",
                    error.what()
                );
            }
            // try to parse & apply processors
            if (!has_failed) {
                try {
                    connection.parse_request();
                    if (!(server._routing && server._routing_processor.process(connection))) {
                        if (!(server._static_processing && server._static_processor.process(connection))) {
                            if (!(server._dynamic_processing && server._dynamic_processor.process(connection))) {
                                connection.response.code = 404;
                                connection.response.data = {
                                    {"message", "Could not find URL on this web server"},
                                    {"details", {
                                        {"code", 404},
                                        {"url", url}
                                    }}
                                };
                            }
                        }
                    }
                } catch (Exceptions::GenericException& error) {
                    connection.response.code = error.get_http_code();
                    connection.response.data = {
                        {"message", error.get_message()},
                        {"details", error.get_details()}
                    };
                } catch (std::exception& error) {
                    connection.response.code = 500;
                    connection.response.data = {
                        {"message", error.what()}
                    };
                }
            }
            // that was it!
            connection.compose_response();
            server.get_logger().debug(
                connection.request.method, " ",
                connection.request.url, " ",
                connection.response.code
            );
            server.get_logger().detail(connection.request.raw.str());
            server.get_logger().detail(Conversion::JSON::serialize(connection.response.data));
            return connection.answer();
        }

    protected:

        virtual const std::string get_logger_name() {
            return "HTTP Server";
        }

    private:
        // microhttpd daemon
        std::vector<MHD_OptionItem> _daemon_options;
        MHD_Daemon* _daemon;
        // options
        uint16_t _port;
        unsigned int _threading;
        unsigned int _timeout;
        size_t _post_buffer_size;
        bool _debug;
        bool _routing;
        bool _static_processing;
        bool _dynamic_processing;
        StaticProcessor _static_processor;
        DynamicProcessor _dynamic_processor;
        RoutingProcessor _routing_processor;
        bool _persistent;
    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__SERVER_HPP
