#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__DBCONTROLLER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__DBCONTROLLER_HPP


#include "DB/Database.hpp"
#include "DB/ORM/Controller.hpp"

#include "../Models/Query.hpp"
#include "./UsersController.hpp"


namespace LinkRbrain::Controllers {


    class DBController : public Logging::Loggable {
    public:

        template <typename ... ParametersTypes>
        DBController(const DB::Type& type, const ParametersTypes& ... parameters) :
            _database(type, parameters...),
            queries(_database),
            users(_database) {}

        DB::ORM::Controller<Models::Query> queries;
        UsersController users;

        inline DB::Connection& get_connection() {
            return _database.get_connection();
        }

    protected:

        virtual const std::string get_logger_name() {
            return "DBController";
        }

    private:

        DB::Database _database;

    };

} // LinkRbrain::Controllers


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__DBCONTROLLER_HPP
