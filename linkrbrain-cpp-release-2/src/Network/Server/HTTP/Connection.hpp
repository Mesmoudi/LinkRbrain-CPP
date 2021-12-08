#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__CONNECTION_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__CONNECTION_HPP


#include "./Request.hpp"
#include "./Response.hpp"

#include "Conversion/JSON.hpp"

#include <microhttpd.h>
#include <map>


namespace Network::Server::HTTP {

    enum Format {
        JSON,
    };

    class Connection {
    public:

        Connection(MHD_Connection* connection, const std::string& method, const std::string& url) :
            _connection(connection),
            _post_processor(NULL),
            request(method, url) {}

        MHD_PostProcessor* get_post_processor() {
            return _post_processor;
        }
        void set_post_processor(MHD_PostProcessor* post_processor) {
            _post_processor = post_processor;
        }

        void parse_request() {
            static std::vector<std::pair<std::string, Format>> content_types = {
                {"application/json", JSON},
                {"text/x-json", JSON},
            };
            auto it = request.headers.find("Content-Type");
            if (it != request.headers.end()) {
                for (const auto& [content_type, buffering_format] : content_types) {
                    if (it->second.find(content_type) != std::string::npos) {
                        switch (buffering_format) {
                            case JSON:
                                Conversion::JSON::parse(request.raw, request.data);
                                return;
                            default:
                                return;
                        }
                    }
                }
            }
        }
        void compose_response() {
            if (response.raw.tellp() == 0  &&  response.data.get_type() != Types::Variant::Undefined) {
                response.headers["Content-Type"] = "application/json";
                Conversion::JSON::serialize(response.raw, response.data);
            }
            if (response.code == 0) {
                if (response.raw.tellp()) {
                    response.code = 200;
                } else {
                    response.code = 204;
                }
            }
        }

        const int answer() {
            // build MHD response
            struct MHD_Response *mhd_response;
            bool is_persistent = false;
            // add response data to MHD response
            const std::string& contents = response.raw.str();
            mhd_response = MHD_create_response_from_buffer(
                contents.size(),
                (void*) contents.data(),
                is_persistent ? MHD_RESPMEM_PERSISTENT : MHD_RESPMEM_MUST_COPY
            );
            // add headers
            for (const auto& header : response.headers) {
                MHD_add_response_header(mhd_response, header.first.c_str(), header.second.c_str());
            }
            // queue response
            int ret = MHD_queue_response(_connection, response.code, mhd_response);
            MHD_destroy_response(mhd_response);
            return ret;
        }

        Request request;
        Response response;

    private:

        MHD_Connection* _connection;
        MHD_PostProcessor* _post_processor;

    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__CONNECTION_HPP
