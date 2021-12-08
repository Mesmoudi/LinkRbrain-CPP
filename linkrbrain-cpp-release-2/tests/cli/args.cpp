#include "CLI/Arguments/Command.hpp"


namespace Commands {

    void hello(const CLI::Arguments::CommandResult& result) {
        std::cout << "Hello " << result.get_value("name") << "!\n";
    }

} // Commands


int main(int argc, char const *argv[]) {
    try {

        CLI::Arguments::Command root("linkrbrain",
            "This program is a dummy thing that will be used as a basis to "
            "write future command-line based programs for LinkRbrain."
        );

        auto& hello = root.add_subcommand("hello", "Says hello", Commands::hello);
        hello.add_option('n', "name", "Name of the person to salute", "world");

        auto& user = root.add_subcommand("user", "Perform operations on LinkRbrain users");
        auto& user_add = user.add_subcommand("add", "Add a user");
        user_add.add_option('u', "username", "User login", CLI::Arguments::Option::Required);
        user_add.add_option('p', "password", "User password", CLI::Arguments::Option::Required);
        user_add.add_option('a', "admin", "User is an administrator", CLI::Arguments::Option::Flag);
        auto& user_remove = user.add_subcommand("remove", "Remove a user");
        auto& user_reset_password = user.add_subcommand("reset_password", "Reset a user's password");

        auto& dataset = root.add_subcommand("dataset", "Perform operations on LinkRbrain datasets");
        auto& dataset_tree = dataset.add_subcommand("list", "View existing data in the form of a tree");
        auto& dataset_add = dataset.add_subcommand("add", "Integrate a new dataset from files");
        auto& dataset_remove = dataset.add_subcommand("remove", "Remove an exiting dataset");
        auto& dataset_correlate = dataset.add_subcommand("correlate", "Correlate points with an existing datset");

        auto& webserver = root.add_subcommand("webserver", "Manage web server");

        const auto result = root.interpret(argc, argv);
        std::cout << "\nInterpreted data:\n";
        result.view();

        // everything went fine
        return 0;


    } catch (const Exceptions::GenericException& error) {

        // handle errors
        std::cout << error.get_message() << '\n';
        return 1;
    }
}
