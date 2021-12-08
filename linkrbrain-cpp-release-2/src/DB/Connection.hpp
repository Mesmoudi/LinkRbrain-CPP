#ifndef LINKRBRAIN2019__SRC__DB__CONNECTION_HPP
#define LINKRBRAIN2019__SRC__DB__CONNECTION_HPP


#include "./Iterator.hpp"
#include "./FieldType.hpp"

#include "Logging/Loggable.hpp"
#include "Types/Variant.hpp"
#include "Conversion/JSON.hpp"

#include <atomic>


namespace DB {


    class Connection : public Logging::Loggable {
    public:

        Connection() : _connection_index(++_connection_counter) {}
        virtual ~Connection() {}
        virtual Iterator execute_parameters(const std::string& sql, const std::vector<std::string>& parameters) = 0;
        virtual Iterator execute(const std::string& sql) = 0;
        virtual void commit() = 0;

        template <typename ...ParametersTypes>
        Iterator execute(const std::string& sql, const ParametersTypes& ... parameters) {
            std::vector<std::string> formatted_parameters;
            convert_parameters(formatted_parameters, parameters...);
            return execute_parameters(sql, formatted_parameters);
        }

        //

        const std::string convert_parameter(const Types::Variant& parameter, const FieldType& type) {
            switch (type) {
                case Variant: {
                    return Conversion::JSON::serialize(parameter);
                }
                case UInt64:
                    return std::to_string(parameter.template get<int64_t>());
                case String:
                    return parameter.get_string();
                case DateTime:
                    return parameter.get_datetime();
                default:
                    except("Not implemented");
            }
        }

        const std::string convert_parameter(const char* parameter) {
            return std::string(parameter);
        }
        const std::string convert_parameter(const std::string& parameter) {
            return parameter;
        }
        const std::string convert_parameter(const Types::Variant& parameter) {
            return Conversion::JSON::serialize(parameter);
        }
        const std::string convert_parameter(const Types::DateTime& parameter) {
            return parameter;
        }
        template <typename T>
        const std::string convert_parameter(const T& parameter) {
            return std::to_string(parameter);
        }

        template <typename T>
        void parse_value(T& destination, const std::string& source) {
            destination = (T) source;
        }
        void parse_value(std::string& destination, const std::string& source) {
            destination = source;
        }
        void parse_value(uint64_t& destination, const std::string& source) {
            destination = std::stoul(source);
        }
        void parse_value(Types::Variant& destination, const std::string& source) {
            Conversion::JSON::parse(source, destination);
        }
        void parse_value(bool& destination, const std::string& source) {
            destination = (source[0] == 't' || source[0] == '1');
        }

        virtual const std::string get_engine_name() = 0;

    protected:

        template <typename ParameterType>
        void convert_parameters(std::vector<std::string>& formatted_parameters, const ParameterType& parameter) {
            formatted_parameters.push_back(
                convert_parameter(parameter)
            );
        }
        template <typename ParameterType, typename ... ParametersTypes>
        void convert_parameters(std::vector<std::string>& formatted_parameters, const ParameterType& parameter, const ParametersTypes& ... parameters) {
            convert_parameters(formatted_parameters, parameter);
            convert_parameters(formatted_parameters, parameters...);
        }

        const size_t compute_new_query_index() {
            return ++_query_counter;
        }
        void log_query(const size_t query_index, const std::string& sql, const std::vector<std::string>& parameters={}) {
            if (get_logger().get_level() > Logging::Level::Debug) {
                return;
            }
            get_logger().debug("Executed query", query_index, ", request:", sql);
            std::string parameters_string;
            if (parameters.size()) {
                bool is_first = true;
                for (const std::string& parameter : parameters) {
                    if (is_first) {
                        is_first = false;
                    } else {
                        parameters_string += " / ";
                    }
                    parameters_string += parameter;
                }
                get_logger().debug("Executed query", query_index, ", parameters:", parameters_string);
            }
        }

        virtual const std::string get_logger_name() {
            return "DB::Connection[" + std::to_string(_connection_index) + "|" + get_engine_name() + "]";
        }

    private:

        const size_t _connection_index;
        static std::atomic<size_t> _connection_counter;
        static std::atomic<size_t> _query_counter;

    };

    std::atomic<size_t> Connection::_connection_counter = 0;
    std::atomic<size_t> Connection::_query_counter = 0;


} // DB


#endif // LINKRBRAIN2019__SRC__DB__CONNECTION_HPP
