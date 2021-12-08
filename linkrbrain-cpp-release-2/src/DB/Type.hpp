#ifndef LINKRBRAIN2019__SRC__DB__TYPE_HPP
#define LINKRBRAIN2019__SRC__DB__TYPE_HPP


#include "Exceptions/GenericExceptions.hpp"

#include <string>


namespace DB {

    enum Type {
        Postgres = 1,
    };

    const Type get_type_from_string(std::string source) {
        // lower the case
        std::transform(source.begin(), source.end(), source.begin(), tolower);
        // check
        if (source == "postgres") {
            return Type::Postgres;
        } else {
            throw Exceptions::BadDataException("Unsupported database type: " + source, {});
        }
    }

} // DB


#endif // LINKRBRAIN2019__SRC__DB__TYPE_HPP
