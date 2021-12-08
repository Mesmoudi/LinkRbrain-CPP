#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__BASEVIEW_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__BASEVIEW_HPP


#include "Network/Server/HTTP/ParametrizedResource.hpp"
#include "LinkRbrain/Controllers/AppController.hpp"


namespace LinkRbrain::Views {

    using namespace Network::Server::HTTP;

    template <typename T>
    class BaseView : public ParametrizedResource<Controllers::AppController<T>> {
    public:

        typedef LinkRbrain::Controllers::AppController<T> AppController;

        using ParametrizedResource<AppController>::ParametrizedResource;

    };


} // LinkRbrain::Views


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__BASEVIEW_HPP
