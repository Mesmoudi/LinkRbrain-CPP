#ifndef CPPP____INCLUDE____CONVERSION__YAML__PARSE_HPP
#define CPPP____INCLUDE____CONVERSION__YAML__PARSE_HPP


#include "Types/Variant.hpp"
#include "../helpers.hpp"
#include "./Parser.hpp"

#include <yaml.h>


namespace Conversion::YAML {

    template<typename T>
    void parse(std::istream& buffer, T& destination);

    template <>
    void parse<Types::Variant>(std::istream& buffer, Types::Variant& destination) {
        Parser parser(buffer);
        parser.parse(destination);
    }

    CPPP____CONVERSION____HELPERS____PARSE

} // Conversion:::YAML


#endif // CPPP____INCLUDE____CONVERSION__YAML__PARSE_HPP
