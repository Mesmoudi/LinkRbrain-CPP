#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__REQUEST_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__REQUEST_HPP


#include "./File.hpp"

#include "Types/Variant.hpp"
#include "Types/MiniMap.hpp"

#include <microhttpd.h>

#include <map>
#include <sstream>


namespace Network::Server::HTTP {


    struct Request {
    public:

        Request(const std::string& _method, const std::string& _url) :
            method(_method),
            url(_url),
            is_uploading(false) {}

        bool is_uploading;
        std::string method;
        std::string url;
        std::vector<std::string> url_parameters;
        Types::MiniMap<std::string, std::string> query;
        Types::MiniMap<std::string, File> files;
        std::unordered_map<std::string, std::string> headers;
        std::stringstream raw;
        std::string ip;
        Types::Variant data;

    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__REQUEST_HPP
