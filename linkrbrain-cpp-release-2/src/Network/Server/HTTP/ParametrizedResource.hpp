#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__PARAMETRIZEDRESOURCE_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__PARAMETRIZEDRESOURCE_HPP


#include <functional>

#include "./BaseResource.hpp"


namespace Network::Server::HTTP {


    template <typename _ParameterType>
    struct ParametrizedResource : public BaseResource {
    public:

        typedef _ParameterType ParameterType;

        ParametrizedResource(const std::string& url, ParameterType& parameter) :
            BaseResource(url),
            _parameter(parameter) {}

        virtual void process(Connection& connection) {
            dispatch(connection);
        }

        virtual void dispatch(Connection& connection) {
            #define NETWORK__SERVER__HTTP__PARAMETERRESOURCE__DISPATCH__METHOD(NAME) \
                if (connection.request.method == #NAME) { \
                    return this->NAME(connection.request, connection.response, _parameter); \
                }
            NETWORK__SERVER__HTTP__PARAMETERRESOURCE__DISPATCH__METHOD(GET)
            NETWORK__SERVER__HTTP__PARAMETERRESOURCE__DISPATCH__METHOD(POST)
            NETWORK__SERVER__HTTP__PARAMETERRESOURCE__DISPATCH__METHOD(PATCH)
            NETWORK__SERVER__HTTP__PARAMETERRESOURCE__DISPATCH__METHOD(DELETE)
            NETWORK__SERVER__HTTP__PARAMETERRESOURCE__DISPATCH__METHOD(PUT)
            #undef NETWORK__SERVER__HTTP__PARAMETERRESOURCE__DISPATCH__METHOD
            generate_405(connection.response);
        }

        #define NETWORK__SERVER__HTTP__PARAMETERRESOURCE__METHOD(NAME) \
            virtual void NAME(const Request& request, Response& response, ParameterType& parameter) { \
                generate_405(response); \
            }

        NETWORK__SERVER__HTTP__PARAMETERRESOURCE__METHOD(GET)
        NETWORK__SERVER__HTTP__PARAMETERRESOURCE__METHOD(POST)
        NETWORK__SERVER__HTTP__PARAMETERRESOURCE__METHOD(PATCH)
        NETWORK__SERVER__HTTP__PARAMETERRESOURCE__METHOD(PUT)
        NETWORK__SERVER__HTTP__PARAMETERRESOURCE__METHOD(DELETE)
        NETWORK__SERVER__HTTP__PARAMETERRESOURCE__METHOD(OPTIONS)

        #undef NETWORK__SERVER__HTTP__PARAMETERRESOURCE__METHOD

    private:

        ParameterType& _parameter;

    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__PARAMETRIZEDRESOURCE_HPP
