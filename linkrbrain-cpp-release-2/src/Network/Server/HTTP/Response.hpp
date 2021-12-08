#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__RESPONSE_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__RESPONSE_HPP


#include "Types/Variant.hpp"

#include <microhttpd.h>

#include <map>
#include <string>
#include <sstream>


namespace Network::Server::HTTP {


    struct Response {
    public:

        Response() :
            code(0) {}

        Response(const Response& source) {
            code = source.code;
            data = source.data;
            raw << source.raw.str();
            headers = source.headers;
        }

        Response& operator= (const Response& source) {
            code = source.code;
            data = source.data;
            raw << source.raw.str();
            headers = source.headers;
            return *this;
        }

        uint16_t code;
        Types::Variant data;
        std::stringstream raw;
        std::unordered_map<std::string, std::string> headers;
    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__RESPONSE_HPP
