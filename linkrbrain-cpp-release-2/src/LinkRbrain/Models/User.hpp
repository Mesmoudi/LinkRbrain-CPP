#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__USER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__USER_HPP


#include "DB/ORM/Controller.hpp"


namespace LinkRbrain::Models {


    struct User {

        size_t id;
        std::string username;
        std::string password;
        Types::DateTime created_at;

        void register_model(DB::ORM::Controller<User>& model_controller) {
            model_controller.set_table_name("users");
            model_controller.register_field(this, id, "id", DB::ORM::Field::Primary | DB::ORM::Field::ReadOnly);
            model_controller.register_field(this, username, "username", DB::ORM::Field::Mandatory);
            model_controller.register_field(this, password, "password", DB::ORM::Field::Mandatory | DB::ORM::Field::WriteOnly);
            model_controller.register_field(this, created_at, "created_at", DB::ORM::Field::ReadOnly);
        }

    };


} // LinkRbrain::Models


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__USER_HPP
