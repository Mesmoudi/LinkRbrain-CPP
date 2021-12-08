#ifndef LINKRBRAIN2019__SRC__NETWORK__SOCKETS__SOCKETCONNECTION_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SOCKETS__SOCKETCONNECTION_HPP


#include <string>
#include <atomic>

#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "Logging/Loggable.hpp"


namespace Network::Sockets {


    class SocketConnection : public Logging::Loggable {
    public:

        SocketConnection(const int socket, const std::string& logger_name) :
            _identifier(++_counter),
            _socket(socket),
            _logger_name(logger_name + " #" + std::to_string(_identifier)) {}

        virtual const bool is_message_complete(const std::string& message) const {
            if (message.size() < sizeof(size_t)) {
                return false;
            }
            if (message.size() < * (size_t*) message.data()) {
                return false;
            }
            return true;
        }

        virtual const std::string read() {
            const std::string message = read_raw();
            if (message.size() < sizeof(size_t)) {
                return "";
            }
            return {message.data() + sizeof(size_t), message.size() - sizeof(size_t)};
        }

        virtual const std::string make_prefix(const std::string& payload) {
            const size_t size = payload.size() + sizeof(size_t);
            return std::string((char*) & size, sizeof(size));
        }
        virtual const std::string make_suffix(const std::string& payload) {
            return "";
        }

        virtual void write(const std::string& payload) {
            write_raw(
                make_prefix(payload) + payload + make_suffix(payload)
            );
        }

    protected:

        const std::string read_raw() {
            std::string message;
            char buffer[1024];
            int size;
            do {
                size = ::recv(_socket, buffer, sizeof(buffer), 0);
                if (size < 0) {
                    get_logger().error("Error while reading from socket: " + std::string(strerror(errno)));
                    break;
                }
                message.append(buffer, size);
                get_logger().debug("Got ", size, " additionnal bytes from socket");
            } while (size > 0 && !is_message_complete(message));
            get_logger().debug("Read all ", message.size(), " bytes from socket");
            return message;
        }
        void write_raw(const std::string& message) {
            if (message.size() == 0) {
                get_logger().debug("Ignoring empty message, nothing will be written");
                return;
            }
            int sent = 0;
            int size;
            do {
                size = ::write(_socket, message.data() + sent, message.size() - sent);
                if (size < 0) {
                    get_logger().error("Error while writing to socket: " + std::string(strerror(errno)));
                    break;
                }
                get_logger().debug("Gave ", size, " bytes to socket");
                sent += size;
            } while (sent < message.size());
            get_logger().debug("Wrote all ", sent, " bytes to socket");
        }

        const std::string get_logger_name() {
            return _logger_name;
        }

    private:

        static std::atomic<size_t> _counter;
        const size_t _identifier;
        const std::string _logger_name;
        const int _socket;

    };

    std::atomic<size_t> SocketConnection::_counter = 0;


} // Network::Sockets


#endif // LINKRBRAIN2019__SRC__NETWORK__SOCKETS__SOCKETCONNECTION_HPP
