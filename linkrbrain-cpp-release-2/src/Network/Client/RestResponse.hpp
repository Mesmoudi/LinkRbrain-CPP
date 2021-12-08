#ifndef LINKRBRAIN2019__SRC__NETWORK__RESTRESPONSE_HPP
#define LINKRBRAIN2019__SRC__NETWORK__RESTRESPONSE_HPP


#include "Types/Variant.hpp"


namespace Network::Client {

    struct RestResponse {
        std::string method;
        std::string url;
        uint16_t code;
        Types::Variant data;
    };

}


#endif // LINKRBRAIN2019__SRC__NETWORK__RESTRESPONSE_HPP
