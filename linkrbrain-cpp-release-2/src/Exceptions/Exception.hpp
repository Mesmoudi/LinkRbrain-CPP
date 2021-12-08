#ifndef LINKRBRAIN2019__SRC__EXCEPTIONS__EXCEPTION_HPP
#define LINKRBRAIN2019__SRC__EXCEPTIONS__EXCEPTION_HPP


#include <exception>
#include <string>
#include <sstream>


namespace Exceptions {


    class Exception : public std::exception {
    public:

        Exception(const std::string& filename, const std::string& function, const size_t line, const std::string& message="") :
            _line(line),
            _function(function),
            _filename(filename),
            _what("Error in method `" + function + "` of `" + filename + "`, line " + std::to_string(line) + (message.size() ? (": " + message) : "")) {}

        Exception(const std::string& message="") : _what(message), _line(-1) {}
        template<typename ...Args>
        Exception(const std::string& filename, const std::string& function, const size_t line, Args const&... args)
            : Exception(filename, function, line, concatenate(args...))
        {}

        virtual const char* what() const noexcept {
            return _what.c_str();
        }

        const std::string& get_filename() const {
            return _filename;
        }
        const std::string& get_function() const {
            return _function;
        }
        const size_t& get_line() const {
            return _line;
        }

    private:

        inline std::string concatenate() {
            return "";
        }
        template<typename T>
        inline std::string concatenate(const T& t) {
            std::stringstream string_stream;
            string_stream << t;
            return string_stream.str();
        }

        template<typename T, typename ... Args>
        inline std::string concatenate(const T& first, Args ... args) {
            return concatenate(first) + " " + concatenate(args...);
        }

        const std::string _what;
        const std::string _filename;
        const std::string _function;
        const size_t _line;

    };

} // Exceptions


#define except(...) {throw Exceptions::Exception(__FILE__, __PRETTY_FUNCTION__, __LINE__, ## __VA_ARGS__);}


#endif // LINKRBRAIN2019__SRC__EXCEPTIONS__EXCEPTION_HPP
