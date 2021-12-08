#include "Network/Sockets/SocketClient.hpp"
#include "Network/Sockets/SocketServer.hpp"
#include "Network/Sockets/SocketConnection.hpp"


// static const std::string host = "perdu.com";
// static const std::string host = "www.example.com";
static Logging::Logger& logger = Logging::get_logger();


void test(const std::string& host_or_path, const int port=-1) {
    // instanciate client & server
    std::shared_ptr<Network::Sockets::SocketClient> client;
    std::shared_ptr<Network::Sockets::SocketServer> server;
    if (port < 0) {
        // UNIX socket
        client = std::make_shared<Network::Sockets::SocketClient>(host_or_path);
        server = std::make_shared<Network::Sockets::SocketServer>(host_or_path);
    } else {
        // IP socket
        client = std::make_shared<Network::Sockets::SocketClient>(host_or_path, port);
        server = std::make_shared<Network::Sockets::SocketServer>(port);
    }
    // make them interact
    server->start();
    std::cout << client->send("Hello world!") << '\n';
    // std::cout << client->send("This is a second test...") << '\n';
    // std::cout << client->send("Last, but not least.") << '\n';
    getchar();
}


int main(int argc, char const *argv[]) {

    // Network::Sockets::SocketClient client(host, 80);
    // std::cout << client.send("GET / HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n") << '\n';

    test("localhost", 8003);
    std::cout << "\n\n";

    test("/tmp/test-socket-" + std::to_string(time(NULL)) + ".sock");

    return 0;
}
