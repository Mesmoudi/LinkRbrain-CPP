#ifndef CPPP____INCLUDE____CONVERSION__JSON__PARSE_HPP
#define CPPP____INCLUDE____CONVERSION__JSON__PARSE_HPP


#include "Types/Variant.hpp"
#include "../helpers.hpp"

#include <json-c/json.h>


namespace Conversion::JSON {

    template<typename T>
    void parse(std::istream& buffer, T& destination);

    void integrate(Types::Variant& value, json_object* obj, bool is_null=false) {
        if (is_null) {
            value.emplace<Types::Variant::Null>();
            return;
        }
        switch (json_object_get_type(obj)) {
            case json_type_null:
                value.emplace<Types::Variant::Null>();
                break;
            case json_type_boolean:
                value.emplace<Types::Variant::Boolean>(json_object_get_boolean(obj));
                break;
            case json_type_double:
                value.emplace<Types::Variant::Floating>(json_object_get_double(obj));
                break;
            case json_type_int:
                value.emplace<Types::Variant::Integer>(json_object_get_int(obj));
                break;
            case json_type_object: {
                value.emplace<Types::Variant::Map>();
                json_object_object_foreach(obj, key, val) {
                    integrate(value.get<Types::Variant::Map>()[key], val);
                }
                break;
            }
            case json_type_array: {
                const int size = json_object_array_length(obj);
                value.emplace<Types::Variant::Vector>();
                value.get<Types::Variant::Vector>().resize(size);
                for (int i=0; i<size; ++i) {
                    integrate(value.get<Types::Variant::Vector>()[i], json_object_array_get_idx(obj, i));
                }
                break;
            }
            case json_type_string: {
                const char* data = json_object_get_string(obj);
                const int size = json_object_get_string_len(obj);
                value.emplace<Types::Variant::String>(data, size);
                break;
            }
        }
    }

    template <>
    void parse<Types::Variant>(std::istream& buffer, Types::Variant& destination) {
        json_tokener* tokener = json_tokener_new();
        json_tokener_error error = json_tokener_continue;
        json_object* obj;
        char low_buffer[256];
        size_t fullsize = 0;//
        while (error == json_tokener_continue) {
            buffer.read(low_buffer, sizeof(low_buffer));
            size_t size = buffer.gcount();
            fullsize += size;
            if (size == 0) {
                break;
            }
            obj = json_tokener_parse_ex(tokener, low_buffer, size);
            error = json_tokener_get_error(tokener);
            if (size < sizeof(low_buffer)) {
                break;
            }
        }
        //
        bool is_null = false;
        if (fullsize == 4 && memcmp(low_buffer, "null", 4) == 0) {
            is_null = true;
        } else if (error != json_tokener_success) {
            Types::Variant details = {
                {"error", json_tokener_error_desc(error)},
                {"offset", tokener->char_offset},
                {"depth", tokener->depth},
                {"st_pos", tokener->st_pos}
            };
            json_tokener_free(tokener);
            throw Exceptions::Exception(std::string("Error while parsing JSON: ") + json_tokener_error_desc(error));
        }
        integrate(destination, obj, is_null);
        //
        json_object_put(obj);
        json_tokener_free(tokener);
    }

    CPPP____CONVERSION____HELPERS____PARSE

} // Conversion:::JSON


#endif // CPPP____INCLUDE____CONVERSION__JSON__PARSE_HPP
