#ifndef CPPP____INCLUDE____CONVERSION__BINARY__PARSE_HPP
#define CPPP____INCLUDE____CONVERSION__BINARY__PARSE_HPP


#include "Types/Variant.hpp"
#include "Types/DateTime.hpp"

#include "../helpers.hpp"


namespace Conversion::Binary {


    template<typename T, std::enable_if_t<!std::is_arithmetic<T>::value && !std::is_enum<T>::value, int> = 0>
    void parse(std::istream& buffer, T& destination);


    // direct parsing

    template<typename T>
    void straight_parse(std::istream& buffer, T& destination, const size_t count=1) {
        buffer.read((char*) &destination, count * sizeof(T));
    }

    // convert numbers & enums

    template<typename T, std::enable_if_t<std::is_arithmetic<T>::value || std::is_enum<T>::value, int> = 0>
    void parse(std::istream& buffer, T& destination) {
        straight_parse(buffer, destination);
    }

    // convert string

    void parse(std::istream& buffer, std::string& destination) {
        size_t size;
        parse(buffer, size);
        destination.resize(size);
        buffer.read((char*) &destination[0], size);
    }

    // format list-like

    template <typename Iterator>
    void iterator_parse(std::istream& buffer, Iterator& iterator) {
        size_t size;
        parse(buffer, size);
        iterator.resize(size);
        for (size_t i = 0; i < size; i++) {
            parse(buffer, iterator[i]);
        }
    }

    // format map-like

    template <typename Map>
    void map_parse(std::istream& buffer, Map& map) {
        size_t size;
        parse(buffer, size);
        std::pair<typename Map::key_type, typename Map::mapped_type> value;
        for (size_t i = 0; i < size; i++) {
            parse(buffer, value.first);
            parse(buffer, value.second);
            map.insert(value);
        }
    }

    // datetime

    template <>
    void parse<Types::DateTime>(std::istream& buffer, Types::DateTime& destination) {
        straight_parse(buffer, destination);
    }

    // variant

    template <>
    void parse<Types::Variant>(std::istream& buffer, Types::Variant& destination) {
        Types::Variant::Type type;
        straight_parse(buffer, type);
        switch (type) {
            case Types::Variant::Undefined:
                destination.emplace<Types::Variant::Null>();
                break;
            case Types::Variant::Null:
                destination.emplace<Types::Variant::Null>();
                break;
            case Types::Variant::Boolean:
                destination.emplace<Types::Variant::Boolean>();
                parse(buffer, destination.get<Types::Variant::Boolean>());
                break;
            case Types::Variant::Character:
                destination.emplace<Types::Variant::Character>();
                parse(buffer, destination.get<Types::Variant::Character>());
                break;
            case Types::Variant::Integer:
                destination.emplace<Types::Variant::Integer>();
                parse(buffer, destination.template get<int64_t>());
                break;
            case Types::Variant::Floating:
                destination.emplace<Types::Variant::Floating>();
                parse(buffer, destination.get<Types::Variant::Floating>());
                break;
            case Types::Variant::String:
                destination.emplace<Types::Variant::String>();
                parse(buffer, destination.get<Types::Variant::String>());
                break;
            case Types::Variant::Vector:
                destination.emplace<Types::Variant::Vector>();
                iterator_parse(buffer, destination.get<Types::Variant::Vector>());
                break;
            case Types::Variant::Map:
                destination.emplace<Types::Variant::Map>();
                map_parse(buffer, destination.get<Types::Variant::Map>());
                break;
            case Types::Variant::DateTime:
                destination.emplace<Types::Variant::DateTime>();
                parse(buffer, destination.get<Types::Variant::DateTime>());
                break;
        }
    }

    // helpers

    CPPP____CONVERSION____HELPERS____PARSE


} // Conversion::Binary


#endif // CPPP____INCLUDE____CONVERSION__BINARY__PARSE_HPP
