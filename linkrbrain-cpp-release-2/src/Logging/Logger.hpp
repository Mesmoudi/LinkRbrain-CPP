#ifndef LINKRBRAIN2019__SRC__LOGGING__LOGGER_HPP
#define LINKRBRAIN2019__SRC__LOGGING__LOGGER_HPP


#include <map>
#include <vector>
#include <memory>
#include <string>
#include <iomanip>
#include <ostream>

#include <sys/time.h>

#include "./Level.hpp"
#include "./Output.hpp"


namespace Logging {

    class Logger {
    public:

        Logger(const std::vector<Output>& outputs, const Level& level, const std::string& name, const bool show_onoff=true) :
            _level(level),
            _name(name),
            _show_onoff(show_onoff),
            _last_millitime(get_millitime())
        {
            set_outputs(outputs);
            if (_show_onoff) {
                log(Log, "Open logger");
            }
        }
        ~Logger() {
            if (_show_onoff) {
                log(Log, "Close logger");
            }
        }

        void set_outputs(const std::vector<Output>& outputs) {
            _outputs.clear();
            for (const Output& output : outputs) {
                switch (output.type) {
                    case Output::StandardError:
                    case Output::StandardOutput:
                        _outputs.push_back(output);
                        break;
                    case Output::File:
                        {
                            Output output2(output.path / (_name + ".log"), true);
                            output2.set_color(output.color);
                            _outputs.push_back(output2);
                        }
                        break;
                }
            }
        }

        const Level& get_level() const {
            return _level;
        }
        void set_level(const Level& level) {
            _level = level;
        }

        inline static double get_millitime() {
            double t;
            timeval tv;
            gettimeofday(&tv, NULL);
            t = (double)tv.tv_sec + 1e-6 * (double)tv.tv_usec;
            return t;
        }

        template<typename ... Args>
        inline void detail(Args ... args) {
            log(Detail, args...);
        }
        template<typename ... Args>
        inline void debug(Args ... args) {
            log(Debug, args...);
        }
        template<typename ... Args>
        inline void notice(Args ... args) {
            log(Notice, args...);
        }
        template<typename ... Args>
        inline void message(Args ... args) {
            log(Message, args...);
        }
        template<typename ... Args>
        inline void warning(Args ... args) {
            log(Warning, args...);
        }
        template<typename ... Args>
        inline void error(Args ... args) {
            log(Error, args...);
        }
        template<typename ... Args>
        inline void fatal(Args ... args) {
            log(Fatal, args...);
        }

    private:

        inline void log_prefix(std::ostream& buffer, const Level level, const bool color) {
            const double dt = get_millitime() - _last_millitime;
            switch (level) {
                case Log:
                    if (color) {
                        buffer << "\x1B[35m"; // magenta
                    }
                    buffer << "Logger  ";
                    break;
                case Detail:
                    if (color) {
                        buffer << "\x1B[37m"; // gray
                    }
                    buffer << "Detail  ";
                    break;
                case Debug:
                    if (color) {
                        buffer << "\x1B[34m"; // blue
                    }
                    buffer << "Debug   ";
                    break;
                case Notice:
                    if (color) {
                        buffer << "\x1B[36m"; // cyan
                    }
                    buffer << "Notice  ";
                    break;
                case Message:
                    if (color) {
                        buffer << "\x1B[32m"; // green
                    }
                    buffer << "Message ";
                    break;
                case Warning:
                    if (color) {
                        buffer << "\x1B[33m"; // yellow
                    }
                    buffer << "Warning ";
                    break;
                case Error:
                    if (color) {
                        buffer << "\x1B[31m"; // red
                    }
                    buffer << "Error   ";
                    break;
                case Fatal:
                    if (color) {
                        buffer << "\x1B[41m"; // white on red
                    }
                    buffer << "Fatal   ";
                    break;
                case None:
                    break;
            };
            std::streamsize default_precision = buffer.precision();
            buffer.setf(std::ios_base::fixed);
            buffer << " | t = ";
            buffer << std::setprecision(6) << get_millitime();
            buffer << " | dt =";
            buffer << std::fixed << std::setprecision(9) << std::right << std::setw(16) << dt;
            buffer.unsetf(std::ios_base::fixed);
            buffer.precision(default_precision);
            buffer << " | ";
            buffer << std::left << std::setw(20) << _name;
            buffer << " | ";
        }
        inline void log_suffix(std::ostream& buffer, const Level level, const bool color) {
            if (color) {
                buffer << "\x1b[0m\x1B[39m";
            }
            buffer << "\n";
        }

        template<typename ... Args>
        inline void log(const Level level, Args ... args) {
            if (level < _level) {
                return;
            }
            for (Output& output : _outputs) {
                if (level < output.level) {
                    return;
                }
                std::ostringstream buffer;
                log_prefix(buffer, level, output.color);
                log_content(buffer, args...);
                log_suffix(buffer, level, output.color);
                output << buffer.str();
            }
            _last_millitime = get_millitime();
        }

        inline void log_content(std::ostream& buffer) {}
        template<typename T>
        inline void log_content(std::ostream& buffer, const T& t) {
            buffer << t;
        }
        template<typename T, typename ... Args>
        inline void log_content(std::ostream& buffer, const T first, Args ... args) {
            log_content(buffer, first);
            log_content(buffer, args...);
        }

        std::string _name;
        std::vector<Output> _outputs;
        bool _show_onoff;
        Level _level;
        double _last_millitime;
    };

} // Logging


#endif // LINKRBRAIN2019__SRC__LOGGING__LOGGER_HPP
