#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__PROCESSOR_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__PROCESSOR_HPP


#include "./Connection.hpp"


namespace Network::Server::HTTP {


    class Processor {
    public:

        virtual const bool process(Connection& connection) = 0;

    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__PROCESSOR_HPP
