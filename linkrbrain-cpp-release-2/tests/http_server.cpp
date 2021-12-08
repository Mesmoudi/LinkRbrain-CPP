#include "Network/Server/HTTP/Server.hpp"
using namespace Network::Server::HTTP;

#include "Logging/Loggers.hpp"
#include "Logging/Loggers.hpp"
static Logging::Logger& logger = Logging::get_logger();


class MethodArgument {
public:
    const std::string test() const {
        return "Hello world";
    }
};


class TestsResource : public BaseResource {
    using BaseResource::BaseResource;
    virtual void GET(const Request& request, Response& response) {
        response.data["id"] = std::stol(request.url_parameters[1]);
        response.data["pi"] = 3.14;
        response.data["foo"] = "bar";
    }
};
class ExperimentsResource : public ParametrizedResource<MethodArgument> {
    using ParametrizedResource::ParametrizedResource;
    virtual void GET(const Request& request, Response& response, MethodArgument& argument) {
        response.data["id"] = std::stol(request.url_parameters[1]);
        response.data["argument"] = argument.test();
    }
};


int main(int argc, char const *argv[]) {
    MethodArgument argument;
    Network::Server::HTTP::Server server;
    server.set_port(8080);
    server.set_threading(16);
    // server.set_routing(false);
    server.set_resource_parameter(argument);
    server.add_resource<TestsResource>("/tests/(\\d+)");
    server.add_resource<ExperimentsResource>("/experiments/(\\d+)");
    server.add_static_path("static");
    server.add_redirection("^/old_test\\d*\\.html$", "/test.html", Redirection::Temporary);
    server.add_redirection("^/new_test\\d*\\.html$", "/test.html", Redirection::Invisible, true);
    server.add_redirection("^/special_test\\d*\\.html$", "/test.html", Redirection::Permanent);
    server.start();
    logger.message("Started server (press ENTER to stop)");
    getc(stdin);
    return 0;
}
