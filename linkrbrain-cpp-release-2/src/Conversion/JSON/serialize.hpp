#ifndef CPPP____INCLUDE____CONVERSION__JSON__SERIALIZE_HPP
#define CPPP____INCLUDE____CONVERSION__JSON__SERIALIZE_HPP


#include "Types/Variant.hpp"
#include "Types/DateTime.hpp"
#include "../helpers.hpp"


namespace Conversion::JSON {

    template <typename T, std::enable_if_t<!std::is_arithmetic<T>::value, int> = 0>
    void serialize(std::ostream& buffer, const T& source);

    // direct conversion

    template<typename T>
    void straight_serialize(std::ostream& buffer, const T& source) {
        buffer << source;
    }

    // format boolean

    void serialize(std::ostream& buffer, const bool& source) {
        buffer << (source ? "true" : "false");
    }

    // format numbers

    template<class T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    void serialize(std::ostream& buffer, const T& source) {
        if (std::isnan(source)) {
            buffer << "NaN";
        } else {
            straight_serialize(buffer, source);
        }
    }
    template<class T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    void serialize(std::ostream& buffer, const T& source) {
        straight_serialize(buffer, source);
    }

    // convert string

    void serialize(std::ostream& buffer, const std::string& source) {
        buffer << '"';
        for (const char c : source) {
            switch (c) {
                case '\0':
                    buffer << "\\0";
                    break;
                case '\t':
                    buffer << "\\t";
                    break;
                case '\f':
                    buffer << "\\f";
                    break;
                case '\b':
                    buffer << "\\b";
                    break;
                case '\r':
                    buffer << "\\r";
                    break;
                case '\n':
                    buffer << "\\n";
                    break;
                case '\\':
                    buffer << "\\\\";
                    break;
                case '"':
                    buffer << "\\\"";
                    break;
                default:
                    buffer << c;
            }
        }
        buffer << '"';
    }

    void serialize(std::ostream& buffer, const char* source) {
        serialize(buffer, std::string(source));
    }

    // format list-like

    template <typename Iterator>
    void iterator_serialize(std::ostream& buffer, const Iterator& iterator) {
        buffer << '[';
        bool is_first = true;
        for (const auto& item : iterator) {
            if (is_first) is_first = false;
            else buffer << ',';
            serialize(buffer, item);
        }
        buffer << ']';
    }

    // format map-like

    template <typename Map>
    void map_serialize(std::ostream& buffer, const Map& map) {
        buffer << '{';
        bool is_first = true;
        for (const auto& [key, value] : map) {
            if (is_first) is_first = false;
            else buffer << ',';
            serialize(buffer, key);
            buffer << ':';
            serialize(buffer, value);
        }
        buffer << '}';
    }

    // datetime

    template <>
    void serialize<Types::DateTime>(std::ostream& buffer, const Types::DateTime& source) {
        buffer << '"' << source << '"';
    }

    // variant

    template <>
    void serialize<Types::Variant>(std::ostream& buffer, const Types::Variant& source) {
        switch (source.get_type()) {
            case Types::Variant::Undefined:
            case Types::Variant::Null:
                buffer << "null";
                break;
            case Types::Variant::Boolean:
                serialize(buffer, source.get<Types::Variant::Boolean>());
                break;
            case Types::Variant::Character:
                serialize(buffer, source.get<Types::Variant::Character>());
                break;
            case Types::Variant::Integer:
                serialize(buffer, source.template get<int64_t>());
                break;
            case Types::Variant::Floating:
                serialize(buffer, source.get<Types::Variant::Floating>());
                break;
            case Types::Variant::String:
                serialize(buffer, source.get<Types::Variant::String>());
                break;
            case Types::Variant::Vector:
                iterator_serialize(buffer, source.get<Types::Variant::Vector>());
                break;
            case Types::Variant::Map:
                map_serialize(buffer, source.get<Types::Variant::Map>());
                break;
            case Types::Variant::DateTime:
                serialize(buffer, source.get<Types::Variant::DateTime>());
                break;
        }
    }

    // helpers

    CPPP____CONVERSION____HELPERS____SERIALIZE


} // Conversion::JSON


#endif // CPPP____INCLUDE____CONVERSION__JSON__SERIALIZE_HPP
