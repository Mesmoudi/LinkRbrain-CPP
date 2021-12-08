#ifndef LINKRBRAIN2019__SRC__LOGGING__LOGGERS_HPP
#define LINKRBRAIN2019__SRC__LOGGING__LOGGERS_HPP


#include "./Logger.hpp"
#include "./Output.hpp"

#include <mutex>
#include <map>
#include <vector>
#include <iostream>
#include <set>


namespace Logging {

    Level _level;

    const Level& get_level() {
        return _level;
    }
    void set_level(const Level& level) {
        _level = level;
    }

    class Loggers {
    public:

        Loggers() :
            _color(false),
            _show_onoff(false),
            _level(Debug) {}

        Logger& get(const std::string& name="") {
            _mutex.lock();
            auto result = _loggers.find(name);
            if (result == _loggers.end()) {
                result = _loggers.insert({name, std::make_shared<Logger>(
                    _outputs, // outputs
                    _level, // level
                    name, // name
                    true // show on/off
                )}).first;
            }
            _mutex.unlock();
            return * result->second;
        }

        void clear_outputs() {
            _outputs.clear();
        }
        template <typename ...T>
        Output& add_output(const T& ...args) {
            _outputs.push_back({args...});
            _outputs.back().set_level(_level);
            return _outputs.back();
        }

        void set_color(const bool color) {
            _color = color;
        }

        void set_level(const Level level) {
            _level = level;
        }

    private:

        std::mutex _mutex;
        std::map<std::string, std::shared_ptr<Logger>> _loggers;
        std::vector<Output> _outputs;
        Level _level;
        bool _color;
        bool _show_onoff;
    };

    static Loggers loggers;

    Logger& get_logger(const std::string& name="") {
        return loggers.get(name);
    }
    template <typename ...T>
    Output& add_output(const T& ...args) {
        return loggers.add_output(args...);
    }

} // Logging


#endif // LINKRBRAIN2019__SRC__LOGGING__LOGGERS_HPP
