#ifndef LINKRBRAIN2019__SRC__CLI__ARGUMENTS_COMMAND_HPP
#define LINKRBRAIN2019__SRC__CLI__ARGUMENTS_COMMAND_HPP



#include "./Option.hpp"

#include "Exceptions/Exception.hpp"
#include "Exceptions/GenericExceptions.hpp"

#include <vector>
#include <string>
#include <iostream>


namespace CLI::Arguments {


    class CommandResult;


    class Command {
    public:

        typedef void Callback(const CommandResult&);

        Command(const std::string& name, const std::string& description, Command* parent=NULL, const bool auto_prompt=true, const bool auto_help=true, Callback* callback=NULL) :
            _name(name),
            _description(description),
            _parent(parent),
            _auto_prompt(auto_help),
            _auto_help(auto_help),
            _callback(callback)
        {
            for (const char c : _name) {
                if (!std::isalnum(c) && c != '_') {
                    throw Exceptions::BadDataException("Command name can only consist of alphanumeric character and underscores", {});
                }
            }
        }

        const bool has_parent() const {
            return (_parent != NULL);
        }
        Command& get_parent() {
            if (_parent == NULL) {
                throw Exceptions::NotFoundException("Could found parent for this command parser: " + _name + " - " + _description, {});
            }
            return *_parent;
        }
        const Command& get_parent() const {
            if (_parent == NULL) {
                throw Exceptions::NotFoundException("Could found parent for this command parser: " + _name + " - " + _description, {});
            }
            return *_parent;
        }

        const std::string& get_name() const { return _name; }
        const std::string& get_description() const { return _description; }

        Option& add_option(const char& short_name, const std::string& long_name, const std::string& description, const int& flags=0, const std::string& fallback="") {
            const Option option{
                short_name,
                long_name,
                description,
                flags,
                fallback
            };
            for (const Option& older_option : _options) {
                if (older_option.conflicts_with(option)) {
                    throw Exceptions::DuplicateException(
                        "This newly defined option...\n" + option.generate_help() + "\n\n...conflicts with the previously defined option below...\n" + older_option.generate_help(),
                    {});
                }
            }
            _options.push_back(option);
            return _options[_options.size() - 1];
        }
        Option& add_option(const char& short_name, const std::string& long_name, const std::string& description, const std::string& fallback) {
            return add_option(short_name, long_name, description, Option::Required | Option::Fallback, fallback);
        }
        const std::vector<Option>& get_options() const {
            return _options;
        }

        Command& add_subcommand(const std::string& name, const std::string& description, Callback* callback=NULL) {
            for (const auto& older_child : _children) {
                if (older_child->get_name() == name) {
                    const std::string message = "This newly defined command...\n" + name + "\n\n...conflicts with the previously defined command below...\n" + older_child->generate_short_help();
                    throw Exceptions::DuplicateException(message, {});
                }
            }
            _children.push_back({});
            _children.rbegin()->reset(new Command(name, description, this, _auto_prompt, _auto_help, callback));
            return ** _children.rbegin();
        }
        const Command& get_subcommand(const std::string& name, const bool auto_help=true) const {
            for (const auto& subcommand : _children) {
                if (subcommand->get_name() == name) {
                    return *subcommand;
                }
            }
            if (auto_help && _auto_help && name == "help") {
                show_help_and_exit();
            }
            throw Exceptions::NotFoundException("Could not find subcommand '" + name +  "' for command '" + generate_full_command_name() + "'", {});
        }
        const bool has_subcommand() const {
            return _children.size();
        }

        const std::string generate_full_command_name(int i=0) const {
            if (has_parent()) {
                return get_parent().generate_full_command_name(i+1) + " " + _name;
            } else {
                return _name;
            }
        }
        const std::string generate_short_help() const {
            return generate_full_command_name() + " - " + _description;
        }
        const std::string generate_help(const size_t& column_width=24) const {
            std::stringstream buffer;
            const std::string fullname = generate_full_command_name();
            // first paragraph: brief description
            buffer << generate_short_help() << "\n\n";
            // second paragraph: usage
            buffer << "Usage: " << fullname << " [<options>]\n";
            buffer << "       " << fullname << " [<command>] [<options>]\n";
            // options paragraph
            if (_options.size() || _auto_help) {
                buffer << "\nAvailable options:\n";
            }
            if (_auto_help) {
                buffer << std::left << std::setw(column_width-1) << "-h, --help" << " Generate this helpful message and exit\n";
            }
            for (const Option& option : _options) {
                buffer << option.generate_help(column_width) << '\n';
            }
            // commands paragraph
            if (_children.size() || _auto_help) {
                buffer << "\nAvailable commands:\n";
            }
            if (_auto_help) {
                buffer << std::left << std::setw(column_width-1) << "help" << " Generate this helpful message and exit\n";
            }
            for (const auto& command : _children) {
                buffer << std::left << std::setw(column_width-1) << command->get_name() << ' ' << command->get_description() << '\n';
            }
            // the end!
            return buffer.str();
        }
        void show_help_and_exit(const size_t& column_width=24, std::ostream& buffer=std::cout) const {
            buffer << generate_help(column_width) << '\n';
            exit(0);
        }

        const CommandResult interpret(int argc, char const *argv[], const bool auto_help=true, const bool auto_prompt=true) const;

        void callback(const CommandResult& result) const {
            if (_callback) {
                return _callback(result);
            }
            return;
        }

    private:

        const std::string _name;
        const std::string _description;
        const bool _auto_prompt;
        const bool _auto_help;
        Command* _parent;
        Callback* _callback;
        std::vector<std::shared_ptr<Command>> _children;
        std::vector<Option> _options;

    };


    class CommandResult {
    public:

        CommandResult(const Command& command, int argc, char const *argv[], CommandResult* parent=NULL, const bool auto_help=true, const bool auto_prompt=true) :
            _command(command),
            _auto_help(auto_help),
            _auto_prompt(auto_prompt),
            _parent(parent)
        {
            // initialize result
            for (const Option& option : command.get_options()) {
                _options_results.push_back({option});
            }
            // browse input argument
            size_t i;
            for (i = 1; i < argc; i++) {
                // basic info
                const char* arg = argv[i];
                const size_t arg_size = strlen(arg);
                if (arg_size == 0) {
                    continue;
                }
                if (arg_size < 2) {
                    throw Exceptions::BadDataException(std::string("Option is too short: ") + std::string(arg), {});
                }
                if (arg[0] != '-') {
                    _child.reset(
                        new CommandResult(command.get_subcommand(argv[i], auto_help), argc-i, argv+i, this, auto_help, auto_prompt)
                    );
                    break;
                }
                // extract name characteristics
                const bool using_short_name = (arg[1] != '-');
                const size_t name_start = using_short_name ? 1 : 2;
                const size_t name_end = (using_short_name && arg[2] != '=') ? 2 : strcspn(arg, "= \t\n");
                const size_t name_size = name_end - name_start;
                std::string name = {arg, name_start, name_size};
                // retrieve corresponding option
                OptionResult& option_result = get_option_result(name);
                // is this a duplicate?
                if (option_result.parsed && !option_result.option.is_multiple()) {
                    throw Exceptions::ConflictException("Multiple occurrences of option " + std::string(arg, name_end) + " are not accepted", {});
                }
                // fetch & record corresponding value
                std::string value;
                if (!option_result.option.is_flag()) {
                    // is the value directly in this arg?
                    if (name_end < arg_size) {
                        if (using_short_name) {
                            value = arg + name_end;
                        } else {
                            value = arg + name_end + 1;
                        }
                    } else if (++i < argc) {
                        value = argv[i];
                        while (*value.rbegin() == '\\' && ++i < argc) {
                            *value.rbegin() = ' ';
                            value += argv[i];
                        }
                    } else {
                        throw Exceptions::BadDataException("Missing value for option " + std::string(arg, name_end), {});
                    }
                }
                option_result.values.push_back(value);
                option_result.parsed = option_result.has_value = true;
            }
            // check if another subcommand is expected
            if (i == argc && _command.has_subcommand()) {
                if (_auto_help) {
                    _command.show_help_and_exit();
                } else {
                    throw Exceptions::BadDataException("Subcommand is expected after '" + _command.generate_full_command_name() + "'", {});
                }
            }
            // check required options and dependencies
            while(!check());
        }
        const std::vector<OptionResult>& get_options_results() const {
            return _options_results;
        }
        const OptionResult& get_option_result(const std::string& name) const {
            if (_auto_help && (name == "h" || name == "help")) {
                _command.show_help_and_exit();
            }
            const bool using_short_name = (name.size() == 1);
            for (const OptionResult& option_result : _options_results) {
                if ((using_short_name && option_result.option.short_name == name[0]) || option_result.option.long_name == name) {
                    return option_result;
                }
            }
            throw Exceptions::NotFoundException("No provided option has the name the developper asked for: " + name, {});
        }

        const bool has(const std::string& name) const {
            const OptionResult& option_result = get_option_result(name);
            return option_result.has_value;
        }
        const std::string& get(const std::string& name) const {
            const OptionResult& option_result = get_option_result(name);
            if (option_result.has_value) {
                return option_result.values[0];
            }
            throw Exceptions::NotFoundException("No value was provided in arguments for option:\n" + option_result.option.generate_help(), {});
        }
        const std::vector<std::string>& get_all(const std::string& name) const {
            const OptionResult& option_result = get_option_result(name);
            if (option_result.has_value || (option_result.option.is_multiple() && !option_result.option.is_required())) {
                return option_result.values;
            }
            throw Exceptions::NotFoundException("No value was provided in arguments for option:\n" + option_result.option.generate_help(), {});
        }

        const bool has_parent() const {
            return (const bool) _parent;
        }
        const CommandResult& get_parent(size_t n=1) const {
            const CommandResult* parent = this;
            while (n--) {
                if (parent->_parent == NULL) {
                    throw Exceptions::NotFoundException("Could not find parent for this result", {});
                }
                parent = parent->_parent;
            }
            return *parent;
        }

        const bool has_child() const {
            return (const bool) _child;
        }
        const CommandResult& get_child(size_t n=1) const {
            const CommandResult* child = this;
            while (n--) {
                if (! (const bool) child->_child) {
                    throw Exceptions::NotFoundException("Could not find child for this result", {});
                }
                child = & * child->_child;
            }
            return *child;
        }

        void callback() const {
            _command.callback(*this);
            if (has_child()) {
                get_child().callback();
            }
        }

        void view(std::ostream& buffer=std::cout, const int level=0) const {
            std::string indent = std::string(4 * level, ' ');
            buffer << indent << "● " << _command.get_name() << '\n';
            indent += "    ";
            for (auto& option_result : _options_results) {
                buffer << indent << option_result.option.short_name << ", " << option_result.option.long_name << ": ";
                if (option_result.has_value) {
                    if (option_result.values.size() && !option_result.option.is_flag()) {
                        buffer << "`";
                        bool first = true;
                        for (const std::string& value : option_result.values) {
                            if (first) {
                                first = false;
                            } else {
                                buffer << "`, `";
                            }
                            buffer << value;
                        }
                        buffer << "`";
                    } else {
                        buffer << "✓";
                    }
                } else {
                    buffer << "✗";
                }
                buffer << "\n";
            }
            if (has_child()) {
                get_child().view(buffer, level+1);
            }
        }

        const Command& get_command() const {
            return _command;
        }

    private:

        OptionResult& get_option_result(const std::string& name) {
            if (_auto_help && (name == "h" || name == "help")) {
                _command.show_help_and_exit();
            }
            const bool using_short_name = (name.size() == 1);
            for (OptionResult& option_result : _options_results) {
                if ((using_short_name && option_result.option.short_name == name[0]) || option_result.option.long_name == name) {
                    return option_result;
                }
            }
            throw Exceptions::NotFoundException("You provided an option that is not available: " + name, {});
        }

        bool check() {
            bool has_prompted = false;
            for (OptionResult& option_result : _options_results) {
                check_required(option_result, has_prompted);
                check_dependency(option_result);
            }
            return !has_prompted;
        }

        const bool has_met_dependency(const OptionResult& option_result) const {
            if (!option_result.option.has_dependency()) {
                return true;
            }
            if (!has(option_result.option.dependency_name)) {
                return false;
            }
            const std::vector<std::string>& values = option_result.option.dependency_values;
            if (values.size()) {
                const std::string value = get(option_result.option.dependency_name);
                if (std::find(values.begin(), values.end(), value) == values.end()) {
                    return false;
                }
            }
            return true;
        }
        void check_dependency(OptionResult& option_result) {
            if (option_result.option.has_dependency() && option_result.has_value) {
                const std::vector<std::string>& values = option_result.option.dependency_values;
                if (!has(option_result.option.dependency_name)) {
                    throw Exceptions::NotFoundException("Option '" + option_result.option.long_name + "' requires the presence of this option:\n" + option_result.option.dependency_name, {});
                } else if (values.size()) {
                    const std::string value = get(option_result.option.dependency_name);
                    if (std::find(values.begin(), values.end(), value) == values.end()) {
                        std::string serialized_values = "'";
                        bool first = true;
                        for (const std::string& value : values) {
                            if (first) {
                                first = false;
                            } else {
                                serialized_values += "', '";
                            }
                            serialized_values += value;
                        }
                        serialized_values += "'";
                        throw Exceptions::NotFoundException("Option '" + option_result.option.long_name + "' requires that the option '" + option_result.option.dependency_name + "' takes one of the following values:\n" + serialized_values, {});
                    }
                }
            }
        }
        const bool is_required(const OptionResult& option_result) const {
            return (option_result.option.is_required() && has_met_dependency(option_result));
        }
        void check_required(OptionResult& option_result, bool& has_prompted) {
            if (is_required(option_result) && !option_result.has_value) {
                if (option_result.option.has_fallback()) {
                    option_result.values.push_back(option_result.option.fallback);
                    option_result.has_value = true;
                } else if (_auto_prompt) {
                    if (!has_prompted) {
                        std::cout << "Missing required option(s) for command '" + _command.generate_full_command_name() + "':\n";
                    }
                    std::cout << option_result.option.generate_help() << ": ";
                    std::string value;
                    std::getline(std::cin, value);
                    option_result.values.push_back(value);
                    option_result.prompted = option_result.has_value = true;
                    has_prompted = true;
                } else {
                    throw Exceptions::NotFoundException("Missing required option for command '" + _command.generate_full_command_name() + "':\n" + option_result.option.generate_help(), {});
                }
            }
        }

        const Command& _command;
        const bool _auto_help;
        const bool _auto_prompt;
        std::vector<OptionResult> _options_results;
        std::shared_ptr<CommandResult> _child;
        CommandResult* _parent;

    };


    const CommandResult Command::interpret(int argc, char const *argv[], const bool auto_help, const bool auto_prompt) const {
        const CommandResult result{*this, argc, argv, NULL, auto_help, auto_prompt};
        result.callback();
        return result;
    }


} // CLI::Arguments


#endif // LINKRBRAIN2019__SRC__CLI__ARGUMENTS_COMMAND_HPP
