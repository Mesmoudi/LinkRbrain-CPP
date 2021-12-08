#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SOCKET__CLIENT_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SOCKET__CLIENT_HPP


#include "Network/Sockets/SocketClient.hpp"
#include "Conversion/Binary.hpp"
#include "./Action.hpp"


namespace LinkRbrain::Socket {


    class Client : public Network::Sockets::SocketClient {
    public:

        Client(const std::filesystem::path& path) : Network::Sockets::SocketClient(path) {
            connect();
        }

        const std::string send_action(const Action& action) {
            std::stringstream buffer;
            buffer.write((const char*) &action, sizeof(Action));
            return send(buffer.str());
        }

    protected:

        virtual const std::string get_logger_name() {
            return "LinkRbrain Socket Client [" + get_description() + "]";
        }

    };


} // LinkRbrain::Socket


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SOCKET__CLIENT_HPP
