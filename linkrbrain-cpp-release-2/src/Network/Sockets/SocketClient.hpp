#ifndef LINKRBRAIN2019__SRC__NETWORK__SOCKETS__SOCKETCLIENT_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SOCKETS__SOCKETCLIENT_HPP


#include "Exceptions/GenericExceptions.hpp"
#include "Logging/Loggable.hpp"
#include "./SocketConnection.hpp"

#include <filesystem>

#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>


namespace Network::Sockets {

    class SocketClient : public Logging::Loggable {
    public:

        enum Type {
            UNIX,
            IP
        };

        SocketClient(const std::string& host, const int port) :
            _type(IP),
            _host(host),
            _port(port),
            _socket(-1) {}

        SocketClient(const std::filesystem::path& path) :
            _type(UNIX),
            _path(path),
            _port(0),
            _socket(-1) {}

        ~SocketClient() {
            disconnect();
        }

        const bool is_connected() const {
            return (_socket > -1);
        }
        void disconnect() {
            if (_socket >= 0) {
                ::close(_socket);
            }
            get_logger().warning("Socket is disconnected from client");
        }
        void connect() {
            get_logger().debug("Connecting client...");
            // prepare things
            memset(&_address, 0, sizeof(_address));
            int socket_domain;
            size_t address_size;
            switch (_type) {
                case IP:
                    // resolve host
                    _server = gethostbyname(_host.c_str());
                    if (_server == NULL) {
                        throw Exceptions::NetworkException("SocketClient error: cannot resolve host `" + _host + "`");
                    }
                    get_logger().debug("Resolved host `" + _host + "` to `" + ::inet_ntoa(*(in_addr*)(_server->h_addr_list[0])) + "`");
                    // other parameters
                    socket_domain = _address.in.sin_family = AF_INET;
                    ::memcpy(&_address.in.sin_addr.s_addr, _server->h_addr, _server->h_length);
                    _address.in.sin_port = ::htons(_port);
                    address_size = sizeof(_address.in);
                    break;
                case UNIX:
                    socket_domain = _address.un.sun_family = AF_UNIX;
                    if (_path.native().size() > sizeof(_address.un.sun_path)) {
                        throw Exceptions::NetworkException("SocketClient error: path is too long for UNIX socket, expected less than " + std::to_string(sizeof(_address.un.sun_path)) + " bytes: " + _path.native());
                    }
                    memcpy(_address.un.sun_path, _path.native().data(), _path.native().size());
                    address_size = sizeof(_address.un);
                    break;
                default:
                    throw Exceptions::NetworkException("SocketClient error: unsupported socket type");
            }
            // connect socket
            _socket = ::socket(socket_domain, SOCK_STREAM, 0);
            if (_socket < 1) {
                throw Exceptions::NetworkException("SocketClient error while opening socket: " + std::string(strerror(errno)));
            }
            get_logger().debug("Socket is open");
            if (::connect(_socket, &_address._, address_size) < 0) {
                throw Exceptions::NetworkException("SocketClient error while connecting socket: " + std::string(strerror(errno)));
            }
            get_logger().message("Client is connected");
        }

        const std::string get_description() const {
            switch (_type) {
                case IP:
                    return _host + ":" + std::to_string(_port);
                case UNIX:
                    return _path;
                default:
                    return "";
            }
        }

        const std::string send(const std::string& payload) {
            // check connection
            if (!is_connected()) {
                connect();
            }
            SocketConnection connection(_socket, get_logger_name());
            connection.write(payload);
            return connection.read();
        }

    protected:

        virtual const std::string get_logger_name() {
            return "SocketClient[" + get_description() + "]";
        }

    private:

        // user parameters
        const Type _type;
        const std::string _host;
        const int _port;
        const std::filesystem::path _path;
        // internals
        int _socket;
        hostent* _server;
        union {
            sockaddr _;
            sockaddr_in in;
            sockaddr_un un;
        } _address;

    };


} // Network::Sockets

#endif // LINKRBRAIN2019__SRC__NETWORK__SOCKETS__SOCKETCLIENT_HPP
