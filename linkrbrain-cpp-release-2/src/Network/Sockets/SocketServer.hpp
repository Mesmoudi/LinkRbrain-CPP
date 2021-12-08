#ifndef LINKRBRAIN2019__SRC__NETWORK__SOCKETS__SOCKETSERVER_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SOCKETS__SOCKETSERVER_HPP


#include "Exceptions/GenericExceptions.hpp"
#include "Logging/Loggable.hpp"
#include "./SocketConnection.hpp"

#include <filesystem>
#include <atomic>
#include <thread>

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


    class SocketServer : public Logging::Loggable {
    public:

        enum Type {
            UNIX,
            IP
        };

        SocketServer(const int port) :
            _type(IP),
            _port(port),
            _timeout(5),
            _socket(-1) {}

        SocketServer(const std::filesystem::path& path) :
            _type(UNIX),
            _path(path),
            _port(0),
            _timeout(5),
            _socket(-1) {}

        ~SocketServer() {
            stop();
        }

        void start() {
            if (!is_connected()) {
                connect();
            }
            _is_running = true;
            _daemon_thread = std::make_shared<std::thread>(daemon, this);
            get_logger().message("Server is started");
        }
        void stop() {
            _is_running = false;
            if (is_connected()) {
                disconnect();
            }
            if (_daemon_thread && _daemon_thread->joinable()) {
                _daemon_thread->join();
                get_logger().debug("Joined daemon process");
            }
            if (_type == UNIX) {
                std::filesystem::remove(_path);
                get_logger().debug("Deleted socket file");
            }
            get_logger().message("Server is stopped");
        }
        void restart() {
            stop();
            start();
        }

        const bool is_connected() const {
            return (_socket > -1);
        }
        void disconnect() {
            _is_running = false;
            if (_socket > -1) {
                ::close(_socket);
                _socket = -1;
            }
            _handled_connections_count = 0;
            get_logger().notice("Server is disconnected");
        }
        void connect() {
            get_logger().debug("Connecting server...");
            // prepare things
            memset(&_address, 0, sizeof(_address));
            int socket_domain;
            switch (_type) {
                case IP:
                    socket_domain = _address.in.sin_family = AF_INET;
                    _address.in.sin_addr.s_addr = INADDR_ANY;
                    _address.in.sin_port = ::htons(_port);
                    _address_size = sizeof(_address.in);
                    break;
                case UNIX:
                    socket_domain = _address.un.sun_family = AF_UNIX;
                    if (_path.size() > sizeof(_address.un.sun_path)) {
                        throw Exceptions::NetworkException("SocketServer error: path is too long for UNIX socket, expected less than " + std::to_string(sizeof(_address.un.sun_path)) + " bytes: " + _path);
                    }
                    memcpy(_address.un.sun_path, _path.data(), _path.size());
                    _address_size = sizeof(_address.un);
                    break;
                default:
                    throw Exceptions::NetworkException("SocketServer error: unsupported socket type");
            }
            _socket = ::socket(socket_domain, SOCK_STREAM | SOCK_NONBLOCK, 0);
            if (_socket < 1) {
                throw Exceptions::NetworkException("SocketServer error while opening socket: " + std::string(strerror(errno)));
            }
            get_logger().debug("Socket is open");
            if (::bind(_socket, &_address._, _address_size) < 0) {
                throw Exceptions::NetworkException("SocketServer error while binding socket: " + std::string(strerror(errno)));
            }
            get_logger().debug("Socket is binded");
            int maximum_queue_length = 3;
            if (::listen(_socket, maximum_queue_length) < 0) {
                throw Exceptions::NetworkException("SocketServer error while listening on socket: " + std::string(strerror(errno)));
            }
            get_logger().debug("Socket is listening");
            // timeval timeout = {.tv_sec=_timeout, .tv_usec=0};
            // if (::setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            //     throw Exceptions::NetworkException("SocketServer error while applying option #1 to socket: " + std::string(strerror(errno)));
            // }
            // if (::setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            //     throw Exceptions::NetworkException("SocketServer error while applying option #2 to socket: " + std::string(strerror(errno)));
            // }
            // get_logger().debug("Socket options are set");
            get_logger().notice("Server is connected");
        }

        virtual const bool partial_callback(const std::string& payload) {
            if (payload.size() == 0) {
                return true;
            }
            if (payload.size() < * (const size_t*) payload.data()) {
                return true;
            }
            return false;
        }

        virtual const std::string callback(const std::string& payload) {
            const std::string message{payload.data() + sizeof(size_t), payload.size() - sizeof(size_t)};
            std::cout << "Server received payload: `" << payload << "`\n";
            return "Your " + std::to_string(payload.size()) + "-bytes payload has been well-received; starts with a '" + payload[0] + "', ends with a '" + *payload.rbegin() + "'";
        }

        const std::string get_description() const {
            switch (_type) {
                case IP:
                    return std::to_string(_port);
                case UNIX:
                    return _path;
                default:
                    return "";
            }
        }

    protected:

        virtual const std::string get_logger_name() {
            return "SocketServer[" + get_description() + "]";
        }

    private:

        static void daemon(SocketServer* socket_server_) {
            SocketServer& socket_server = *socket_server_;
            socket_server.get_logger().message("Server daemon is running and accepting connections");
            while (socket_server._is_running) {
                int client_socket = ::accept(socket_server._socket, &socket_server._address._, &socket_server._address_size);
                if (client_socket < 0) {
                    if (errno == EAGAIN) {
                        std::this_thread::sleep_for(
                            std::chrono::microseconds(1)
                        );
                        continue;
                    }
                    socket_server.get_logger().error("SocketServer error while accepting on socket: " + std::string(strerror(errno)));
                    continue;
                }
                socket_server.get_logger().debug("Accepted incoming connection");
                ++socket_server._handled_connections_count;
                socket_server._callback(client_socket);
                --socket_server._handled_connections_count;
            }
        }

        void _callback(int client_socket) {
            SocketConnection connection{client_socket, get_logger_name()};
            const std::string request = connection.read();
            const std::string response = callback(request);
            connection.write(response);
        }

        // user parameters
        const Type _type;
        const int _port;
        const std::string _path;
        const uint16_t _timeout;
        // internals
        int _socket;
        union {
            sockaddr _;
            sockaddr_in in;
            sockaddr_un un;
        } _address;
        socklen_t _address_size;
        //
        std::atomic<bool> _is_running;
        std::atomic<size_t> _handled_connections_count;
        std::shared_ptr<std::thread> _daemon_thread;

    };


} // Network::Sockets

#endif // LINKRBRAIN2019__SRC__NETWORK__SOCKETS__SOCKETSERVER_HPP
