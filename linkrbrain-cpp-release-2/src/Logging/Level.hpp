#ifndef LINKRBRAIN2019__SRC__LOGGING__LEVEL_HPP
#define LINKRBRAIN2019__SRC__LOGGING__LEVEL_HPP


#include <map>
#include <string>


namespace Logging {

    enum Level {
        Log = -1,
        Detail = 0,
        Debug = 1,
        Notice = 2,
        Message = 3,
        Warning = 4,
        Error = 5,
        Fatal = 6,
        None = 7,
    };

    static std::map<std::string, Logging::Level> levels_by_name = {
        {"log", Level::Log},
        {"detail", Level::Detail},
        {"debug", Level::Debug},
        {"notice", Level::Notice},
        {"message", Level::Message},
        {"warning", Level::Warning},
        {"error", Level::Error},
        {"fatal", Level::Fatal},
        {"none", Level::None},
    };

    const int get_level_by_name(std::string level_name) {
        // put name to lower case
        std::transform(level_name.begin(), level_name.end(), level_name.begin(),
            [](unsigned char c){ return std::tolower(c); });
        // assign when a valid name has been given
        auto it = levels_by_name.find(level_name);
        if (it == levels_by_name.end()) {
            return -1;
        }
        return it->second;
    }

} // Logging

#endif // LINKRBRAIN2019__SRC__LOGGING__LEVEL_HPP
