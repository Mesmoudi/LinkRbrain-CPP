#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__WEBSERVER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__WEBSERVER_HPP


#include "LinkRbrain/Controllers/AppController.hpp"
#include "LinkRbrain/Socket/Server.hpp"
#include "LinkRbrain/Socket/Client.hpp"

#include "CLI/Arguments/Command.hpp"

#include "./linkrbrain.hpp"


namespace LinkRbrain::Commands {

    void linkrbrain_webserver_start(const CLI::Arguments::CommandResult& options) {
        // initialize application controller
        LinkRbrain::Controllers::AppController<T> app(
            options.get_parent().get("socket"),
            data_path,
            DB::get_type_from_string(options.get_parent().get_parent().get("db-type")),
            options.get_parent().get_parent().get("db-connection")
        );
        // start!
        std::cout << "Starting webserver...\n";
        app.start(true);
        app.get_http_controller().get_server().set_server_caching(options.has("server-caching"));
        app.get_http_controller().get_server().set_client_caching(options.has("client-caching"));
        std::cout << "Started webserver on port " << app.get_http_controller().get_server().get_port() << ", press ENTER to stop\n";
        getc(stdin);
    }

    void linkrbrain_webserver(const CLI::Arguments::CommandResult& options) {
        // Extract parameters
        const std::filesystem::path socket_path = options.get_parent().get("socket");
        const std::string action_name = options.get_command().get_name();
        const LinkRbrain::Socket::Action action = LinkRbrain::Socket::get_action_from_name(action_name);
        // Is it a start?
        if (action_name == "start") {
            bool found_server = false;
            if (std::filesystem::is_socket(socket_path)) {
                try {
                    LinkRbrain::Socket::Client socket(socket_path);
                    socket.send_action(LinkRbrain::Socket::Action::Status);
                    found_server = true;
                } catch (const Exceptions::NetworkException&) {
                    std::cout << "Could not connect to socket" << '\n';
                }
            }
            if (!found_server) {
                if (std::filesystem::exists(socket_path)) {
                    std::filesystem::remove(socket_path);
                    std::cout << "Deleted zombie socket file" << '\n';
                }
                linkrbrain_webserver_start(options);
                return;
            }
        }
        // Perform request
        std::cout << "Requesting action from server: " << LinkRbrain::Socket::get_action_name(action) << "...\n";
        LinkRbrain::Socket::Client client_socket(socket_path);
        const std::string reply = client_socket.send_action(action);
        std::cout << "Server replied: " << reply << '\n';
    }

} // LinkRbrain::Commands


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__WEBSERVER_HPP
