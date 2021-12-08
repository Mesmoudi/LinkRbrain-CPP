#ifndef LINKRBRAIN2019__SRC__MAPPED__EXCEPTIONS_HPP
#define LINKRBRAIN2019__SRC__MAPPED__EXCEPTIONS_HPP


#include "Exceptions/Exception.hpp"


namespace std {
    const std::string& to_string(const std::string& value) {
        return value;
    }
} // std


namespace Paged {

    struct FileSystemException : public Exceptions::Exception {
        inline FileSystemException(const std::string& path, const std::string& action, const std::string& details)
        : Exception("Failed to " + action + ": `" + path + "` (" + details + ")") {}
        inline FileSystemException(const std::string& path, const std::string& action)
        : FileSystemException(path, action, strerror(errno)) {}
    };

    struct BadHeaderException : public Exceptions::Exception {
        inline BadHeaderException()
        : Exception("Bad header") {}
        inline BadHeaderException(const std::string& filename)
        : Exception("Bad header for `" + filename + "`") {}
        template<typename value1_t, typename value2_t>
        inline BadHeaderException(const std::string& filename, const std::string& valuename, value1_t value1, value2_t value2)
        : Exception("Bad header for `" + filename + "`: expected `" + std::to_string(value1) + "` for `" + valuename + "`, got `" + std::to_string(value2) + "`") {}
    };

} // Paged


#endif // LINKRBRAIN2019__SRC__MAPPED__EXCEPTIONS_HPP
