#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__REDIRECTION_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__REDIRECTION_HPP


#include "./Processor.hpp"

#include <vector>


namespace Network::Server::HTTP {


    struct Redirection {
        enum Type {
            Permanent = 301,
            Temporary = 302,
            Invisible = 0,
        };
        std::regex pattern;
        std::string replacement;
        Type type;
        const bool is_last;
    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__REDIRECTION_HPP
