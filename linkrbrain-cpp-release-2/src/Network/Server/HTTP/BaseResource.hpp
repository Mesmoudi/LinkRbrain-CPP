#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__BASERESOURCE_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__BASERESOURCE_HPP


#include <regex>

#include "./Connection.hpp"


namespace Network::Server::HTTP {


    struct BaseResource {
    public:

        typedef void ParameterType;

        BaseResource(const std::string& url) : _regex_url(url) {}
        virtual ~BaseResource() {}

        const bool match(Connection& connection) const {
            // does it match?
            std::smatch matches;
            if (!std::regex_match(connection.request.url, matches, _regex_url)) {
                return false;
            }
            // extract matches within path
            connection.request.url_parameters.resize(matches.size());
            int index = 0;
            for (const auto& match : matches) {
                connection.request.url_parameters[index++] = match;
            }
            // that was it!
            return true;
        }

        virtual void process(Connection& connection) {
            dispatch(connection);
        }

        virtual void dispatch(Connection& connection) {
            #define NETWORK__SERVER__HTTP__BASERESOURCE__DISPATCH__METHOD(NAME) \
                if (connection.request.method == #NAME) { \
                    return this->NAME(connection.request, connection.response); \
                }
            NETWORK__SERVER__HTTP__BASERESOURCE__DISPATCH__METHOD(GET)
            NETWORK__SERVER__HTTP__BASERESOURCE__DISPATCH__METHOD(POST)
            NETWORK__SERVER__HTTP__BASERESOURCE__DISPATCH__METHOD(PATCH)
            NETWORK__SERVER__HTTP__BASERESOURCE__DISPATCH__METHOD(DELETE)
            NETWORK__SERVER__HTTP__BASERESOURCE__DISPATCH__METHOD(PUT)
            #undef NETWORK__SERVER__HTTP__BASERESOURCE__DISPATCH__METHOD
            generate_405(connection.response);
        }

        #define NETWORK__SERVER__HTTP__BASERESOURCE__METHOD(NAME) \
            virtual void NAME(const Request& request, Response& response) { \
                generate_405(response); \
            }

        NETWORK__SERVER__HTTP__BASERESOURCE__METHOD(GET)
        NETWORK__SERVER__HTTP__BASERESOURCE__METHOD(POST)
        NETWORK__SERVER__HTTP__BASERESOURCE__METHOD(PATCH)
        NETWORK__SERVER__HTTP__BASERESOURCE__METHOD(PUT)
        NETWORK__SERVER__HTTP__BASERESOURCE__METHOD(DELETE)
        NETWORK__SERVER__HTTP__BASERESOURCE__METHOD(OPTIONS)

        #undef NETWORK__SERVER__HTTP__BASERESOURCE__METHOD

    protected:

        void generate_405(Response& response) {
            response.code = 405;
            response.data["message"] = "method not allowed";
        }

        const std::regex _regex_url;

    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__BASERESOURCE_HPP
