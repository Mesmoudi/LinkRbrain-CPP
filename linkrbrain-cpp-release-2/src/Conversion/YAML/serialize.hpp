#ifndef CPPP____INCLUDE____CONVERSION__YAML__SERIALIZE_HPP
#define CPPP____INCLUDE____CONVERSION__YAML__SERIALIZE_HPP


#include "Types/Variant.hpp"
#include "Types/DateTime.hpp"
#include "../helpers.hpp"


namespace Conversion::YAML {

    template <typename T, std::enable_if_t<!std::is_arithmetic<T>::value, int> = 0>
    void serialize(std::ostream& buffer, const T& source, const size_t depth = 0);

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
    void serialize(std::ostream& buffer, const T& source, const size_t depth = 0) {
        if (std::isnan(source)) {
            buffer << "NaN";
        } else {
            straight_serialize(buffer, source);
        }
    }
    template<class T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    void serialize(std::ostream& buffer, const T& source, const size_t depth = 0) {
        straight_serialize(buffer, source);
    }

    // convert string

    void serialize(std::ostream& buffer, const std::string& source, const size_t depth = 0) {
        buffer << source;
    }

    void serialize(std::ostream& buffer, const char* source, const size_t depth = 0) {
        serialize(buffer, std::string(source), depth);
    }

    // format list-like

    template <typename Iterator>
    void iterator_serialize(std::ostream& buffer, const Iterator& iterator, const size_t depth = 0) {
        if (iterator.size() == 0) {
            buffer << "[]";
            return;
        }
        const std::string prefix = "\n" + std::string(4 * depth, ' ') + "- ";
        for (const auto& item : iterator) {
            buffer << prefix;
            serialize(buffer, item, depth + 1);
        }
    }

    // format map-like

    template <typename Map>
    void map_serialize(std::ostream& buffer, const Map& map, const size_t depth = 0) {
        if (map.size() == 0) {
            buffer << "{}";
            return;
        }
        const std::string prefix = "\n" + std::string(4 * depth, ' ');
        for (const auto& [key, value] : map) {
            buffer << prefix;
            serialize(buffer, key, depth);
            buffer << ": ";
            serialize(buffer, value, depth + 1);
        }
    }

    // datetime

    template <>
    void serialize<Types::DateTime>(std::ostream& buffer, const Types::DateTime& source, const size_t depth) {
        buffer << source;
    }

    // variant

    template <>
    void serialize<Types::Variant>(std::ostream& buffer, const Types::Variant& source, const size_t depth) {
        switch (source.get_type()) {
            case Types::Variant::Undefined:
            case Types::Variant::Null:
                buffer << "null";
                break;
            case Types::Variant::Boolean:
                serialize(buffer, source.get<Types::Variant::Boolean>(), depth);
                break;
            case Types::Variant::Character:
                serialize(buffer, source.get<Types::Variant::Character>(), depth);
                break;
            case Types::Variant::Integer:
                serialize(buffer, source.template get<int64_t>());
                break;
            case Types::Variant::Floating:
                serialize(buffer, source.get<Types::Variant::Floating>(), depth);
                break;
            case Types::Variant::String:
                serialize(buffer, source.get<Types::Variant::String>(), depth);
                break;
            case Types::Variant::Vector:
                iterator_serialize(buffer, source.get<Types::Variant::Vector>(), depth);
                break;
            case Types::Variant::Map:
                map_serialize(buffer, source.get<Types::Variant::Map>(), depth);
                break;
            case Types::Variant::DateTime:
                serialize(buffer, source.get<Types::Variant::DateTime>(), depth);
                break;
        }
    }

    // helpers

    CPPP____CONVERSION____HELPERS____SERIALIZE


} // Conversion::YAML


#endif // CPPP____INCLUDE____CONVERSION__YAML__SERIALIZE_HPP
