#ifndef LINKRBRAIN2019__SRC__DB__CONNECT_HPP
#define LINKRBRAIN2019__SRC__DB__CONNECT_HPP


#include "./Type.hpp"
#include "./Postgres/PostgresConnection.hpp"


namespace DB {

    template <typename ... ParametersTypes>
    Connection* connect(const Type& type, const ParametersTypes& ... parameters) {
        switch (type) {
            case Postgres:
                return new PostgresConnection(parameters...);
            default:
                except("unrecognized engine type:", type);
        }
    }

} // DB


#endif // LINKRBRAIN2019__SRC__DB__CONNECT_HPP
