#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SOCKET__SERVER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SOCKET__SERVER_HPP


#include "LinkRbrain/Controllers/AppController.hpp"
#include "Network/Sockets/SocketServer.hpp"
#include "Conversion/Binary.hpp"
#include "./Action.hpp"


namespace LinkRbrain::Socket {


    template <typename T>
    class Server : public Network::Sockets::SocketServer {
    public:

        Server(const std::filesystem::path& path, LinkRbrain::Controllers::AppController<T>& app_controller) :
            Network::Sockets::SocketServer(path),
            _app_controller(app_controller) {}

        virtual const std::string callback(const std::string& payload) {
            if (payload.size() < sizeof(Action)) {
                return "invalid request";
            }
            std::stringstream buffer{payload};
            Action action;
            Conversion::Binary::straight_parse(buffer, action);
            try {
                std::cout << "Requested '" << get_action_name(action) << "' via " << get_description() << '\n';
                switch (action) {
                    case None:
                        return "nothing to be done.";
                    case Status:
                        return _app_controller.get_status_name();
                    case Stop:
                        _app_controller.stop();
                        return "stopped server";
                    case Start:
                        _app_controller.start();
                        return "started server";
                    case Restart:
                        _app_controller.restart();
                        return "restarted server";
                }
                return "WTF?";
            } catch (const std::exception& error) {
                std::cerr << "Client request '" << get_action_name(action) << "' via " << get_description() << " FAILED: " << error.what() << '\n';
                return std::string("ERROR: ") + error.what();
            }
        }

    protected:

        virtual const std::string get_logger_name() {
            return "LinkRbrain Socket Server [" + get_description() + "]";
        }

    private:

        LinkRbrain::Controllers::AppController<T>& _app_controller;

    };


} // LinkRbrain::Socket


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SOCKET__SERVER_HPP
