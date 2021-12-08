#ifndef CPPP____INCLUDE____CONVERSION__YAML__PARSER_HPP
#define CPPP____INCLUDE____CONVERSION__YAML__PARSER_HPP


#include "Types/Variant.hpp"
#include "Logging/Loggable.hpp"
#include "Exceptions/GenericExceptions.hpp"

#include <yaml.h>
#include <istream>


namespace Conversion::YAML {


    class Parser : public Logging::Loggable {
    public:

        Parser(std::istream& input_buffer, const std::string& logger_name="") :
            _input_buffer(input_buffer),
            _logger_name(logger_name)
        {
            yaml_parser_initialize(&_yaml_parser);
            yaml_parser_set_input(&_yaml_parser, yaml_input_callback, this);
        }
        ~Parser() {
            yaml_parser_delete(&_yaml_parser);
        }

        void parse(Types::Variant& destination) {
            destination.unset();
            yaml_token_t yaml_token;
            //
            std::vector<Types::Variant*> hierarchy = {&destination};
            size_t depth = 0;
            //
            bool expecting_key = false;
            bool has_key = false;
            std::string key;
            //
            do {
                yaml_parser_scan(&_yaml_parser, &yaml_token);
                if (_yaml_parser.error != YAML_NO_ERROR) {
                    yaml_error();
                }
                switch (yaml_token.type) {
                    case YAML_NO_TOKEN:
                    case YAML_STREAM_START_TOKEN:
                    case YAML_STREAM_END_TOKEN:
                    case YAML_VERSION_DIRECTIVE_TOKEN:
                    case YAML_TAG_DIRECTIVE_TOKEN:
                    case YAML_DOCUMENT_START_TOKEN:
                    case YAML_DOCUMENT_END_TOKEN:
                        break;
                    case YAML_ALIAS_TOKEN:
                    case YAML_ANCHOR_TOKEN:
                        get_logger().warning("Got unhandled 'alias' or 'anchor' token, please read https://blog.daemonl.com/2016/02/yaml.html and implement it in Conversion::YAML");
                        break;
                    case YAML_TAG_TOKEN:
                        get_logger().warning("Got unhandled 'tag' token, please read https://camel.readthedocs.io/en/latest/yamlref.html#tags and implement it in Conversion::YAML");
                        break;
                    case YAML_BLOCK_SEQUENCE_START_TOKEN:
                    case YAML_FLOW_SEQUENCE_START_TOKEN:
                        hierarchy[depth]->set_vector();
                        ++depth;
                        hierarchy.resize(depth + 1);
                        hierarchy[depth] = NULL;
                        break;
                    case YAML_BLOCK_MAPPING_START_TOKEN:
                    case YAML_FLOW_MAPPING_START_TOKEN:
                        hierarchy[depth]->set_map();
                        ++depth;
                        hierarchy.resize(depth + 1);
                        hierarchy[depth] = NULL;
                        break;
                    case YAML_BLOCK_END_TOKEN:
                    case YAML_FLOW_SEQUENCE_END_TOKEN:
                    case YAML_FLOW_MAPPING_END_TOKEN:
                        --depth;
                        hierarchy.resize(depth + 1);
                        break;
                    case YAML_BLOCK_ENTRY_TOKEN:
                    case YAML_FLOW_ENTRY_TOKEN:
                        hierarchy[depth] = & hierarchy[depth - 1]->push_back();
                        break;
                    case YAML_KEY_TOKEN:
                        expecting_key = true;
                        has_key = false;
                        break;
                    case YAML_VALUE_TOKEN:
                        if (has_key) {
                            has_key = false;
                            expecting_key = false;
                            hierarchy[depth] = & (* hierarchy[depth - 1])[key];
                        }
                        break;
                    case YAML_SCALAR_TOKEN:
                        if (expecting_key) {
                            expecting_key = false;
                            has_key = true;
                            key = (char*)yaml_token.data.scalar.value;
                            if (hierarchy[depth - 1]->has(key)) {
                                get_logger().warning("Beware duplicate key at depth ", depth, ": ", key);
                            }
                        } else {
                            Types::Variant* current_destination = hierarchy[depth];
                            if (current_destination == NULL) {
                                if (hierarchy[depth-1]->get_type() == Types::Variant::Vector) {
                                    current_destination = & hierarchy[depth-1]->push_back();
                                } else {
                                    get_logger().error("Destination for scalar is NULL");
                                }
                            }
                            if (yaml_token.data.scalar.style == YAML_PLAIN_SCALAR_STYLE) {
                                parse_scalar((char*)yaml_token.data.scalar.value, * current_destination);
                            } else {
                                current_destination->set_string((char*)yaml_token.data.scalar.value);
                            }
                        }
                        break;
                }
                // next
                if (yaml_token.type != YAML_STREAM_END_TOKEN) {
                    yaml_token_delete(&yaml_token);
                }
            } while (yaml_token.type != YAML_STREAM_END_TOKEN);
            yaml_token_delete(&yaml_token);
        }

        virtual const std::string get_logger_name() {
            if (_logger_name.size()) {
                return "YAML::Parser{" + _logger_name + "}";
            }
            return "YAML::Parser";
        }

    private:

        void yaml_error() {
            throw Exceptions::BadDataException("Error while parsing YAML: " + std::string(_yaml_parser.problem), {
                {"problem", _yaml_parser.problem ? Types::Variant(_yaml_parser.problem) : Types::Variant::Null},
                {"context", _yaml_parser.context ? Types::Variant(_yaml_parser.context) : Types::Variant::Null},
                {"offset", _yaml_parser.problem_offset},
                {"line", _yaml_parser.problem_mark.line},
                {"column", _yaml_parser.problem_mark.column},
            });
        }

        void parse_scalar(const std::string& source, Types::Variant& destination) {
            if (source == "true") {
                destination.set_boolean(true);
            } else if (source == "false") {
                destination.set_boolean(false);
            } else if (source != "null") {
                try {
                    size_t n = 0;
                    const int64_t result = std::stoll(source, &n);
                    if (n == source.size()) {
                        destination.set_integer(result);
                    } else {
                        try {
                            const double result = std::stod(source, &n);
                            if (n == source.size()) {
                                destination.set_floating(result);
                            } else {
                                destination.set_string(source);
                            }
                        } catch (const std::invalid_argument&) {
                            destination.set_string(source);
                        }
                    }
                } catch (const std::invalid_argument&) {
                    destination.set_string(source);
                }
            }
        }

        int yaml_input_callback(char* data, const size_t size, size_t& size_read) {
            size_read = _input_buffer.readsome(data, size);
            get_logger().debug("Requested ", size, " bytes, got ", size_read);
            get_logger().detail("Requested ", size, " bytes, got ", size_read, std::string(data, size_read));
            return 1;
        }
        static int yaml_input_callback(void* parser_, unsigned char* buffer, size_t size, size_t* size_read) {
            Parser& parser = * (Parser*) parser_;
            return parser.yaml_input_callback((char*)buffer, size, *size_read);
        }

        std::istream& _input_buffer;
        std::string _logger_name;
        yaml_parser_t _yaml_parser;

    };


} // Conversion::YAML


#endif // CPPP____INCLUDE____CONVERSION__YAML__PARSER_HPP
