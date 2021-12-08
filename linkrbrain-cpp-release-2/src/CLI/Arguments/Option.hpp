#ifndef LINKRBRAIN2019__SRC__CLI__ARGUMENTS_OPTION_HPP
#define LINKRBRAIN2019__SRC__CLI__ARGUMENTS_OPTION_HPP


#include <vector>
#include <string>

#include "Exceptions/GenericExceptions.hpp"


namespace CLI::Arguments {

    class Option {
    public:

        enum Flags {
            Multiple = 1,
            Required = 2,
            Fallback = 4,
            Flag = 8,
            Hidden = 16,
        };

        Option(
            const char& _short_name, const std::string& _long_name, const std::string& _description,
            const int& _flags, const std::string& _fallback
        ) :
            short_name(_short_name),
            long_name(_long_name),
            description(_description),
            flags(_flags),
            fallback(_fallback)
        {
            // control input
            if (!std::isalnum(short_name)) {
                throw Exceptions::BadDataException("Option short name can only consist of alphanumeric character", {});
            }
            if (long_name.size() < 2) {
                throw Exceptions::BadDataException("Option long name should be at least two characters long", {});
            }
            for (size_t i = 0; i < long_name.size(); i++) {
                char c = long_name[i];
                if (!std::isalnum(c) && c != '-') {
                    throw Exceptions::BadDataException("Option long name can only consist of alphanumeric characters and dashes", {});
                }
                if (c == '-' && (i == 0 || i == long_name.size() - 1)) {
                    throw Exceptions::BadDataException("Option long name cannot start nor end with a dash", {});
                }
            }
            if (long_name.find("--") != std::string::npos) {
                throw Exceptions::BadDataException("Option long name cannot contain two dashes in a row", {});
            }
        }

        const bool has_dependency() const {
            return dependency_name.size();
        }
        const bool is_multiple() const {
            return flags & Flags::Multiple;
        }
        const bool is_required() const {
            return flags & Flags::Required;
        }
        const bool has_fallback() const {
            return flags & Flags::Fallback;
        }
        const bool is_flag() const {
            return flags & Flags::Flag;
        }
        const bool is_hidden() const {
            return flags & Flags::Hidden;
        }

        const bool conflicts_with(const Option& other) const {
            return other.short_name == short_name || other.long_name == long_name;
        }

        const std::string generate_help(const size_t& column_width=24) const {
            if (is_hidden()) {
                return "";
            }
            std::stringstream buffer;
            const std::string names = "-" + std::string(&short_name, 1) + ", --" + long_name;
            buffer << std::left << std::setw(column_width-1) << names << ' ';
            buffer << description;
            std::vector<std::string> specifics;
            if (has_dependency()) {
                if (dependency_values.size() == 0) {
                    specifics.push_back(std::string("depends on the presence of -") + ((dependency_name.size()==1)?"":"-") + dependency_name);
                } else {
                    std::string specificity = "depends on -";
                    specificity += (dependency_name.size()==1) ? "" : "-";
                    specificity += dependency_name + " taking ";
                    if (dependency_values.size() == 1) {
                        specificity += "the value '" + dependency_values[0] + "'";
                    } else {
                        specificity += "one of the values: '";
                        bool first = true;
                        for (const std::string& value : dependency_values) {
                            if (first) {
                                first = false;
                            } else {
                                specificity += "', '";
                            }
                            specificity += value;

                        }
                        specificity += "'";
                    }
                    specifics.push_back(specificity);
                }
            }
            if (is_required() && !has_fallback()) {
                specifics.push_back("required argument");
            }
            if (is_flag()) {
                specifics.push_back("flag");
            }
            if (is_multiple()) {
                specifics.push_back("can be multiple");
            }
            if (has_fallback()) {
                specifics.push_back("defaults to '" + fallback + "'");
            }
            if (specifics.size()) {
                buffer << " (";
                for (size_t i = 0; i < specifics.size(); i++) {
                    if (i != 0) {
                        buffer << ", ";
                    }
                    buffer << specifics[i];
                }
                buffer << ')';
            }
            return buffer.str();
        }

        void depends_on(const std::string& name, const std::vector<std::string>& values) {
            dependency_name = name;
            dependency_values = values;
        }
        void depends_on(const std::string& name, const std::string& value) {
            dependency_name = name;
            dependency_values = {value};
        }
        void depends_on(const std::string& name) {
            dependency_name = name;
        }

        char short_name;
        std::string long_name;
        std::string description;
        int flags;
        std::string fallback;
        std::string dependency_name;
        std::vector<std::string> dependency_values;

    };


    struct OptionResult {
    public:

        OptionResult(const Option& _option) :
            option(_option),
            parsed(false),
            prompted(false),
            has_value(false) {}

        const Option& option;
        bool parsed;
        bool prompted;
        bool has_value;
        std::vector<std::string> values;
    };


} // CLI::Arguments


#endif // LINKRBRAIN2019__SRC__CLI__ARGUMENTS_OPTION_HPP
