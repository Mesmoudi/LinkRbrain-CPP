#ifndef LINKRBRAIN2019__SRC__EXCEPTIONS__DATAEXCEPTIONS_HPP
#define LINKRBRAIN2019__SRC__EXCEPTIONS__DATAEXCEPTIONS_HPP


#include <exception>
#include <memory>
#include <string>


#include "Types/Variant.hpp"


namespace Exceptions {

    class GenericException : public std::exception {
    public:

        GenericException(const std::string& message, const Types::Variant& details={});

        virtual const char* what() const noexcept {
            return _what.c_str();
        }

        virtual const int get_http_code() const { return 500; }

        const std::string& get_message() const {
            return _message;
        }

        const Types::Variant& get_details() const {
            return *_details;
        }

    private:

        std::string _what;
        const std::string _message;
        std::shared_ptr<const Types::Variant> _details;

    };

    #define EXCEPTIONS__BASEEXCEPTIONS__DEFINE(NAME, CODE) \
        struct NAME : public GenericException { \
            using GenericException::GenericException; \
            virtual const int get_http_code() const { return CODE; } \
        };


    EXCEPTIONS__BASEEXCEPTIONS__DEFINE(NotFoundException, 404)
    EXCEPTIONS__BASEEXCEPTIONS__DEFINE(DuplicateException, 409)
    EXCEPTIONS__BASEEXCEPTIONS__DEFINE(ConflictException, 409)
    EXCEPTIONS__BASEEXCEPTIONS__DEFINE(ForbiddenException, 403)
    EXCEPTIONS__BASEEXCEPTIONS__DEFINE(BadDataException, 400)
    EXCEPTIONS__BASEEXCEPTIONS__DEFINE(UnauthorizedException, 401)
    EXCEPTIONS__BASEEXCEPTIONS__DEFINE(DatabaseException, 500)
    EXCEPTIONS__BASEEXCEPTIONS__DEFINE(NotImplementedException, 418)
    EXCEPTIONS__BASEEXCEPTIONS__DEFINE(NetworkException, 503)

} // Exceptions


Exceptions::GenericException::GenericException(const std::string& message, const Types::Variant& details) :
    _message(message),
    _details(new Types::Variant(details)),
    _what(message)
{
    // if (*_details != Types::Variant::Undefined)  {
    //     _what += "; ";
    //     std::stringstream buffer(_what);
    //     _details->serialize(buffer, Buffering::Format::JSON);
    // }
}


#endif // LINKRBRAIN2019__SRC__EXCEPTIONS__DATAEXCEPTIONS_HPP
