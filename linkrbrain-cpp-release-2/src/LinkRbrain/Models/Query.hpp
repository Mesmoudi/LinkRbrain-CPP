#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__QUERY_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__QUERY_HPP


#include "DB/ORM/Controller.hpp"


namespace LinkRbrain::Models {


    struct Query {

        size_t id;
        std::string name;
        Types::Variant groups;
        Types::Variant settings;
        Types::Variant permissions;
        Types::DateTime created_at;
        Types::DateTime updated_at;
        bool is_computed;
        Types::Variant correlations;
        Types::Variant graph;

        void register_model(DB::ORM::Controller<Query>& model_controller) {
            model_controller.set_table_name("queries");
            model_controller.register_field(this, id, "id", DB::ORM::Field::Primary | DB::ORM::Field::ReadOnly);
            model_controller.register_field(this, name, "name", DB::ORM::Field::Mandatory);
            model_controller.register_field(this, groups, "groups", DB::ORM::Field::Mandatory);
            model_controller.register_field(this, settings, "settings", DB::ORM::Field::Mandatory);
            model_controller.register_field(this, permissions, "permissions", DB::ORM::Field::Mandatory);
            model_controller.register_field(this, created_at, "created_at", DB::ORM::Field::ReadOnly);
            model_controller.register_field(this, updated_at, "updated_at", DB::ORM::Field::ReadOnly);
            model_controller.register_field(this, is_computed, "is_computed");
            model_controller.register_field(this, correlations, "correlations");
            model_controller.register_field(this, graph, "graph");
        }

    };


} // LinkRbrain::Models


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__QUERY_HPP
