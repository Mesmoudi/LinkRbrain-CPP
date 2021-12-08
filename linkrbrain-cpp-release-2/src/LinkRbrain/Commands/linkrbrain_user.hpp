#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__USER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__USER_HPP


#include "LinkRbrain/Controllers/DBController.hpp"
#include "CLI/Arguments/Command.hpp"
#include "Types/Table.hpp"

#include "./linkrbrain.hpp"


namespace LinkRbrain::Commands {

    void user(const CLI::Arguments::CommandResult& options) {
        _init_db_controller(options);
    } // user

    void user_list(const CLI::Arguments::CommandResult& options) {
        Types::Table table("id", "username", "creation date");
        for (auto& user : _get_db_controller().users.fetch_all()) {
            table.add_row(user.id, user.username, user.created_at);
        }
        std::cout << table << '\n';
    } // user_list

    void user_add(const CLI::Arguments::CommandResult& options) {
        LinkRbrain::Models::User user({
            .username = options.get("username"),
            .password = options.get("password")});
        _get_db_controller().users.insert(user);
    } // user_add

    void user_remove(const CLI::Arguments::CommandResult& options) {
        const size_t user_id = std::stoi(options.get("identifier"));
        LinkRbrain::Models::User user = _get_db_controller().users.fetch(user_id);
        _get_db_controller().users.remove(user);
        std::cout << "Successfully removed user `" << user.username << "` created at `" << user.created_at << "`\n";
    } // user_remove

    void user_test_password(const CLI::Arguments::CommandResult& options) {
        _get_db_controller().users.fetch_by_credentials(
            options.get("username"),
            options.get("password"));
        std::cout << "Credentials are valid\n";
    } // user_test_password

    void user_reset_password(const CLI::Arguments::CommandResult& options) {
        const size_t user_id = std::stoi(options.get("identifier"));
        LinkRbrain::Models::User user = _get_db_controller().users.fetch(user_id);
        user.password = options.get("password");
        _get_db_controller().users.update(user, {"password"});
        std::cout << "Successfully updated password for user `" << user.username << "` created at `" << user.created_at << "`\n";
    } // user_reset_password

} // LinkRbrain::Commands

#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__USER_HPP
