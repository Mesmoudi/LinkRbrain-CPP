#ifndef LINKRBRAIN2019__SRC__DB__ORM__CONTROLLER_HPP
#define LINKRBRAIN2019__SRC__DB__ORM__CONTROLLER_HPP


#include "../Database.hpp"
#include "./Field.hpp"


namespace DB::ORM {


    template <typename Model>
    class Iterator;


    template <typename Model>
    class Controller {
    public:

        Controller(DB::Database& database) : _database(database) {
            Model instance;
            instance.register_model(*this);
        }

        template <typename ...Args>
        Model fetch(Args ... args) {
            std::string sql;
            //
            for (size_t i=0, n=_fields.size(); i<n ;++i) {
                sql += i ? ", " : "SELECT ";
                sql += _fields[i].name;
            }
            //
            sql += " FROM " + _table_name;
            //
            for (size_t i=0, n=_primary_fields.size(); i<n ;++i) {
                sql += i ? " AND " : " WHERE ";
                sql += _primary_fields[i].name;
                sql += " = $";
                sql += std::to_string(i + 1);
            }
            //
            Model instance;
            DB::Iterator iterator = _database.get_connection().execute(sql, args...).begin();
            if (!iterator) {
                Types::Variant serialized_primary;
                auto serialize_primary = [&] (auto && input) {
                    serialized_primary.push_back(input);
                };
                (serialize_primary(args), ...);
                throw Exceptions::NotFoundException("Cannot fetch resource from " + _table_name, {
                    {"resource", _table_name},
                    {"action", "fetch"},
                    {"primary", serialized_primary}
                });
            }
            set_instance(instance, * iterator.get_cursor());
            return instance;
        }
        Iterator<Model> fetch_all();

        void insert(Model& instance) {
            std::string sql1 = "INSERT INTO " + _table_name;
            std::string sql2 = " VALUES ";
            std::vector<std::string> parameters;
            //
            for (size_t i=0, n=_fields.size(); i<n ;++i) {
                const Field& field = _fields[i];
                if (!field.is_readonly()) {
                    if (parameters.size() == 0) {
                        sql1 += " (";
                        sql2 += " (";
                    } else {
                        sql1 += ", ";
                        sql2 += ", ";
                    }
                    sql1 += field.name;
                    sql2 += "$";
                    sql2 += std::to_string(parameters.size() + 1);
                    parameters.push_back(field.get_text_value(instance));
                }
            }
            //
            sql1 += ")";
            for (size_t i=0, n=_fields.size(); i<n ;++i) {
                sql2 += i ? ", " : ") RETURNING ";
                sql2 += _fields[i].name;
            }
            //
            const std::string sql = sql1 + sql2;
            set_instance(
                instance,
                * _database.get_connection().execute_parameters(sql, parameters).begin().get_cursor()
            );
        }
        Model insert_data(const Types::Variant& data) {
            if (data.get_type() != Types::Variant::Map) {
                throw Exceptions::BadDataException("Data should be presented in a map-type object", {
                    {"resource", _table_name},
                    {"action", "insert"},
                    {"problem", "wrongcontainer"}
                });
            }
            std::string sql1 = "INSERT INTO " + _table_name;
            std::string sql2 = " VALUES ";
            std::vector<std::string> parameters;
            //
            for (const Field& field : _fields) {
                if (!field.is_mandatory()) {
                    continue;
                }
                bool found_key = false;
                for (const auto& [key, value] : data.get_map()) {
                    if (field.name == key) {
                        found_key = true;
                        break;
                    }
                }
                if (!found_key) {
                    throw Exceptions::BadDataException("Field `" + field.name + "` for `" + _table_name + "` is missing", {
                        {"resource", _table_name},
                        {"action", "insert"},
                        {"field", field.name},
                        {"problem", "missingkey"}
                    });
                }
            }
            //
            for (const auto& [key, value] : data.get_map()) {
                bool found_key = false;
                for (const Field& field : _fields) {
                    if (field.name == key) {
                        found_key = true;
                        if (field.is_readonly()) {
                            throw Exceptions::BadDataException("Field " + key + " for " + _table_name + " is read only", {
                                {"resource", _table_name},
                                {"action", "insert"},
                                {"field", key},
                                {"problem", "readonly"}
                            });
                        }
                        if (parameters.size() == 0) {
                            sql1 += " (";
                            sql2 += " (";
                        } else {
                            sql1 += ", ";
                            sql2 += ", ";
                        }
                        sql1 += field.name;
                        sql2 += "$";
                        sql2 += std::to_string(parameters.size() + 1);
                        parameters.push_back(field.get_text_value(value));
                        break;
                    }
                }
                if (!found_key) {
                    throw Exceptions::BadDataException("Field `" + key + "` for `" + _table_name + "` does not exist", {
                        {"resource", _table_name},
                        {"action", "insert"},
                        {"field", key},
                        {"problem", "wrongkey"}
                    });
                }
            }
            //
            sql1 += ")";
            for (size_t i=0, n=_fields.size(); i<n ;++i) {
                sql2 += i ? ", " : ") RETURNING ";
                sql2 += _fields[i].name;
            }
            //
            Model instance;
            const std::string sql = sql1 + sql2;
            set_instance(
                instance,
                * _database.get_connection().execute_parameters(sql, parameters).begin().get_cursor()
            );
            return instance;
        }

        void remove(Model& instance) {
            std::vector<std::string> parameters;
            std::string sql = "DELETE FROM ";
            sql += _table_name;
            for (size_t i=0, n=_primary_fields.size(); i<n ;++i) {
                const Field& field = _primary_fields[i];
                sql += i ? " AND " : " WHERE ";
                sql += field.name;
                sql += " = $";
                sql += std::to_string(i + 1);
                parameters.push_back(field.get_text_value(instance));
            }
            _database.get_connection().execute_parameters(sql, parameters).begin();
        }

        void update(Model& instance, const std::vector<std::string>& fields_names={}) {
            std::vector<std::string> parameters;
            std::string sql = "UPDATE ";
            sql += _table_name;
            //
            for (size_t i=0, n=_fields.size(); i<n ;++i) {
                const Field& field = _fields[i];
                if (!field.is_primary() && !field.is_readonly() && (fields_names.size() == 0 || std::find(fields_names.begin(), fields_names.end(), field.name) != fields_names.end())) {
                    sql += parameters.size() ? ", " : " SET ";
                    sql += field.name;
                    sql += " = $";
                    sql += std::to_string(parameters.size() + 1);
                    parameters.push_back(field.get_text_value(instance));
                }
            }
            //
            for (size_t i=0, n=_primary_fields.size(); i<n ;++i) {
                const Field& field = _primary_fields[i];
                sql += i ? " AND " : " WHERE ";
                sql += field.name;
                sql += " = $";
                sql += std::to_string(parameters.size() + 1);
                parameters.push_back(field.get_text_value(instance));
            }
            //
            for (size_t i=0, n=_fields.size(); i<n ;++i) {
                sql += i ? ", " : " RETURNING ";
                sql += _fields[i].name;
            }
            //
            set_instance(
                instance,
                * _database.get_connection().execute_parameters(sql, parameters).begin().get_cursor()
            );
        }
        void update_data(Model& instance, const Types::Variant& data) {
            if (data.get_type() != Types::Variant::Map) {
                throw Exceptions::BadDataException("Data should be presented in a map-type object", {
                    {"resource", _table_name},
                    {"action", "insert"},
                    {"problem", "wrongcontainer"}
                });
            }
            if (data.get_map().size() == 0) {
                return;
            }
            //
            std::vector<std::string> parameters;
            std::string sql = "UPDATE ";
            sql += _table_name;
            //
            for (const auto& [key, value] : data.get_map()) {
                bool found_key = false;
                for (const Field& field : _fields) {
                    if (field.name == key) {
                        found_key = true;
                        if (field.is_primary()) {
                            throw Exceptions::BadDataException("Field `" + key + "` for " + _table_name + " is primary, cannot be overwritten", {
                                {"resource", _table_name},
                                {"action", "update"},
                                {"field", key},
                                {"problem", "primary"}
                            });
                        }
                        if (field.is_readonly()) {
                            throw Exceptions::BadDataException("Field " + key + " for " + _table_name + " is read only, cannot be overwritten", {
                                {"resource", _table_name},
                                {"action", "update"},
                                {"field", key},
                                {"problem", "readonly"}
                            });
                        }
                        sql += parameters.size() ? ", " : " SET ";
                        sql += field.name;
                        sql += " = $";
                        sql += std::to_string(parameters.size() + 1);
                        parameters.push_back(field.get_text_value(value));
                    }
                }
                if (!found_key) {
                    throw Exceptions::BadDataException("Field `" + key + "` for " + _table_name + " does not exist", {
                        {"resource", _table_name},
                        {"action", "update"},
                        {"field", key},
                        {"problem", "wrongkey"}
                    });
                }
            }
            //
            for (size_t i=0, n=_primary_fields.size(); i<n ;++i) {
                const Field& field = _primary_fields[i];
                sql += i ? " AND " : " WHERE ";
                sql += field.name;
                sql += " = $";
                sql += std::to_string(parameters.size() + 1);
                parameters.push_back(field.get_text_value(instance));
            }
            //
            for (size_t i=0, n=_fields.size(); i<n ;++i) {
                sql += i ? ", " : " RETURNING ";
                sql += _fields[i].name;
            }
            //
            set_instance(
                instance,
                * _database.get_connection().execute_parameters(sql, parameters).begin().get_cursor()
            );
        }

        const size_t count() {
            const std::string sql = "SELECT COUNT(*) FROM " + _table_name + ";";
            DB::Iterator iterator = _database.get_connection().execute(sql).begin();
            if (!iterator) {
                throw Exceptions::Exception("Cannot fetch count for table " + _table_name);
            }
            return std::stoul((*iterator).get_text(0));
        }

        void serialize(Types::Variant& destination, const Model& instance) {
            for (const Field& field : _fields) {
                if (!field.is_writeonly()) {
                    field.serialize(destination, instance);
                }
            }
        }

    protected:

        friend Iterator<Model>;
        void set_instance(Model& instance, DB::Cursor& cursor) {
            for (size_t i=0, n=_fields.size(); i<n ;++i) {
                _fields[i].set_instance_value(instance, cursor.get_text(i));
            }
        }

        DB::Database& _database;

        std::string _table_name;
        std::vector<Field> _fields;
        std::vector<Field> _primary_fields;

    private:

        friend Model;
        void set_table_name(const std::string& table_name) {
            _table_name = table_name;
        }
        template <typename T>
        void register_field(const void* instance_pointer, const T& field_reference, const std::string& field_name, const int traits=0) {
            if (_table_name.size() == 0) {
                except("Table name should be set before fields");
            }
            //
            FieldType field_type = get_field_type<T>();
            if (field_type == Unrecognized) {
                except("Unrecognized field type for field `" + field_name + "` in table `" + _table_name + "`");
            }
            //
            Field field = {
                .resource_name = _table_name,
                .name = field_name,
                .offset = (const char*)&field_reference - (const char*)instance_pointer,
                .type = field_type,
                .traits = traits,
                .database = _database,
            };
            _fields.push_back(field);
            if (field.is_primary()) {
                _primary_fields.push_back(field);
            }
        }

    };


} // DB::ORM


#include "./Iterator.hpp"


namespace DB::ORM {


    template <typename Model>
    Iterator<Model> Controller<Model>::fetch_all() {
        std::string sql;
        //
        for (size_t i=0, n=_fields.size(); i<n ;++i) {
            sql += i ? ", " : "SELECT ";
            sql += _fields[i].name;
        }
        //
        sql += " FROM " + _table_name;
        //
        return {*this, _database.get_connection().execute(sql)};
    }


} // DB::ORM


#endif // LINKRBRAIN2019__SRC__DB__ORM__CONTROLLER_HPP
