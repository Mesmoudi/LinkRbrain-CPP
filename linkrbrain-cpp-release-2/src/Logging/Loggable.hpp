#ifndef LINKRBRAIN2019__SRC__LOGGING__LOGGABLE_HPP
#define LINKRBRAIN2019__SRC__LOGGING__LOGGABLE_HPP


#include "./Logger.hpp"
#include "./Loggers.hpp"

#include <string>


namespace Logging {

    class Loggable {
    public:

        Logger& get_logger() {
            if (!_logger_reference.logger) {
                _logger_reference.logger = & Logging::get_logger(get_logger_name());
            }
            return * _logger_reference.logger;
        }

    protected:

        virtual const std::string get_logger_name() = 0;

    private:

        struct LoggerReference {
            LoggerReference() : logger(NULL) {}
            Logger* logger;
        };

        LoggerReference _logger_reference;

    };

} // Logging


#endif // LINKRBRAIN2019__SRC__LOGGING__LOGGABLE_HPP
