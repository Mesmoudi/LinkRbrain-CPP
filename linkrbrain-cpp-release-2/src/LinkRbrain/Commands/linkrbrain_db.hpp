#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__DB_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__DB_HPP


#include "CLI/Arguments/Command.hpp"
#include "Types/Table.hpp"

#include "./linkrbrain.hpp"


namespace LinkRbrain::Commands {

    void db(const CLI::Arguments::CommandResult& options) {
        _init_db_controller(options);
    } // db

    void db_list(const CLI::Arguments::CommandResult& options) {
        Types::Table table("Table name", "Rows count");
        table.add_row("users", _get_db_controller().users.count());
        table.add_row("queries", _get_db_controller().queries.count());
        std::cout << table << '\n';
    } // db_list

} // LinkRbrain::Commands

#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__DB_HPP
