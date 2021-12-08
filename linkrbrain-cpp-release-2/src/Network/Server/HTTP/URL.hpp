#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__SERVER_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__SERVER_HPP


#include <microhttpd.h>


namespace Network::Server::HTTP {


    class URL {
    public:


    private:
        const uint16_t _port;
        struct MHD_Daemon* _daemon;
    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__SERVER_HPP
