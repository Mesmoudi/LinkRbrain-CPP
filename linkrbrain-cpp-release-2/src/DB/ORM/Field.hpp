#ifndef LINKRBRAIN2019__SRC__DB__ORM__FIELD_HPP
#define LINKRBRAIN2019__SRC__DB__ORM__FIELD_HPP


#include "../FieldType.hpp"
#include "Exceptions/GenericExceptions.hpp"


namespace DB::ORM {


    struct Field {

        enum Trait {
            None = 0,
            Primary = 1,
            Mandatory = 2,
            ReadOnly = 4,
            WriteOnly = 8,
        };

        std::string resource_name;
        std::string name;
        size_t offset;
        FieldType type;
        int traits;
        Database& database;

        const bool is_primary() const {
            return traits & Primary;
        }
        const bool is_mandatory() const {
            return traits & Mandatory;
        }
        const bool is_readonly() const {
            return traits & ReadOnly;
        }
        const bool is_writeonly() const {
            return traits & WriteOnly;
        }

        template <typename Model>
        const std::string get_text_value(const Model& instance) const {
            const char* pointer = (const char*) &instance + offset;
            switch (type) {
                case UInt64:
                    return database.get_connection().convert_parameter(*(size_t*) pointer);
                case String:
                    return database.get_connection().convert_parameter(*(std::string*) pointer);
                case Variant:
                    return database.get_connection().convert_parameter(*(Types::Variant*) pointer);
                case DateTime:
                    return database.get_connection().convert_parameter(*(Types::DateTime*) pointer);
                default:
                    except("Unrecognized field type");
            }
        }
        const std::string get_text_value(const Types::Variant& value) const {
            switch (type) {
                case UInt64:
                    if (value.get_type() != Types::Variant::Integer) {
                        break;
                    }
                    return database.get_connection().convert_parameter(value.template get<int64_t>());
                case String:
                    if (value.get_type() != Types::Variant::String) {
                        break;
                    }
                    return database.get_connection().convert_parameter(value.get_string());
                case Variant:
                    return database.get_connection().convert_parameter(value);
                case DateTime:
                    if (value.get_type() != Types::Variant::String && value.get_type() != Types::Variant::DateTime) {
                        break;
                    }
                    if (value.get_type() == Types::Variant::String) {
                        return database.get_connection().convert_parameter(value.get_string());
                    }
                    return database.get_connection().convert_parameter(value.get_datetime());
                case Boolean:
                    return value.get_boolean() ? "true" : "false";
                case Unrecognized:
                    except("Unrecognized field type");
            }
            //
            throw Exceptions::BadDataException("Invalid data type for field `" + name + "`", {
                {"resource", resource_name},
                {"field", name},
                {"problem", "wrongtype"},
                {"type", value.get_type_name()}
            });
        }

        template <typename Model>
        void serialize(Types::Variant& destination, const Model& instance) const {
            Types::Variant& data = destination[name];
            const char* pointer = (const char*) &instance + offset;
            switch (type) {
                case UInt64:
                    data.set_integer(*(uint64_t*) pointer);
                    break;
                case String:
                    data.set_string(*(std::string*) pointer);
                    break;
                case Variant:
                    data = *(Types::Variant*) pointer;
                    break;
                case DateTime:
                    data.set_datetime(*(Types::DateTime*) pointer);
                    break;
                case Boolean:
                    data.set_boolean(*(bool*) pointer);
                    break;
                case Unrecognized:
                    except("Unrecognized field type");
            }
        }

        template <typename Model>
        void set_instance_value(Model& instance, const std::string& text) {
            const char* pointer = (const char*) &instance + offset;
            switch (type) {
                case UInt64:
                    return database.get_connection().parse_value(*(uint64_t*) pointer, text);
                case String:
                    return database.get_connection().parse_value(*(std::string*) pointer, text);
                case Variant:
                    return database.get_connection().parse_value(*(Types::Variant*) pointer, text);
                case DateTime:
                    return database.get_connection().parse_value(*(Types::DateTime*) pointer, text);
                case Boolean:
                    return database.get_connection().parse_value(*(bool*) pointer, text);
                case Unrecognized:
                    except("Unrecognized field type");
            }
        }

    };


} // DB::ORM


#endif // LINKRBRAIN2019__SRC__DB__ORM__FIELD_HPP
