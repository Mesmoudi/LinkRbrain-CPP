#ifndef CPPP____INCLUDE____CONVERSION__BINARY__SERIALIZE__HPP
#define CPPP____INCLUDE____CONVERSION__BINARY__SERIALIZE__HPP


#include "Types/Variant.hpp"
#include "Types/DateTime.hpp"

#include "../helpers.hpp"


namespace Conversion::Binary {

    template<typename T, std::enable_if_t<!std::is_arithmetic<T>::value && !std::is_enum<T>::value, int> = 0>
    void serialize(std::ostream& buffer, const T& source);

    // direct conversion

    template<typename T>
    void straight_serialize(std::ostream& buffer, const T& source, const size_t count=1) {
        buffer.write((const char*) &source, count * sizeof(T));
    }

    // format numbers

    template<typename T, std::enable_if_t<std::is_arithmetic<T>::value || std::is_enum<T>::value, int> = 0>
    void serialize(std::ostream& buffer, const T& source) {
        straight_serialize(buffer, source);
    }

    // convert string

    void serialize(std::ostream& buffer, const std::string& source) {
        serialize(buffer, source.size());
        buffer.write(source.data(), source.size());
    }

    // format list-like

    template <typename Iterator>
    void iterator_serialize(std::ostream& buffer, const Iterator& iterator) {
        serialize(buffer, iterator.size());
        for (const auto& item : iterator) {
            serialize(buffer, item);
        }
    }

    // format map-like

    template <typename Map>
    void map_serialize(std::ostream& buffer, const Map& iterator) {
        serialize(buffer, iterator.size());
        for (const auto& [key, value] : iterator) {
            serialize(buffer, key);
            serialize(buffer, value);
        }
    }

    // datetime

    template <>
    void serialize<Types::DateTime>(std::ostream& buffer, const Types::DateTime& source) {
        straight_serialize(buffer, source);
    }

    // variant

    template <>
    void serialize<Types::Variant>(std::ostream& buffer, const Types::Variant& source) {
        straight_serialize(buffer, source.get_type());
        switch (source.get_type()) {
            case Types::Variant::Undefined:
            case Types::Variant::Null:
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


} // Conversion::Binary


#endif // CPPP____INCLUDE____CONVERSION__BINARY__SERIALIZE__HPP
