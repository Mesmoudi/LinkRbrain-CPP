#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__HTTPCONTROLLER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__HTTPCONTROLLER_HPP


#include "Network/Server/HTTP/Server.hpp"

#include "LinkRbrain/Views/Upload.hpp"
#include "LinkRbrain/Views/Points.hpp"
#include "LinkRbrain/Views/Organs.hpp"
#include "LinkRbrain/Views/Datasets.hpp"
#include "LinkRbrain/Views/Groups.hpp"
#include "LinkRbrain/Views/Queries.hpp"
#include "LinkRbrain/Views/QueriesPdf.hpp"
#include "LinkRbrain/Views/Users.hpp"
#include "LinkRbrain/Views/Tokens.hpp"
#include "LinkRbrain/Views/Test.hpp"


namespace LinkRbrain::Controllers {

    template <typename T>
    class AppController;

    template <typename T>
    class HTTPController : public Logging::Loggable {
    public:

        HTTPController(AppController<T>& app_controller) {
            _server.set_port(8080);
            _server.set_threading(1);
            // _server.set_threading(std::thread::hardware_concurrency());
            _server.add_static_path("var/www");
            _server.add_static_path("tmp/pdf");
            _server.set_resource_parameter(app_controller);
            // file upload
            _server.add_resource<LinkRbrain::Views::Upload<T>>("/upload-target");
            _server.add_resource<LinkRbrain::Views::Upload<T>>("/api/uploads");
            _server.add_resource<LinkRbrain::Views::Points<T>>("/api/points");
            // organs to dataset to group
            _server.add_resource<LinkRbrain::Views::Organs<T>>("/api/organs/(\\d+)");
            _server.add_resource<LinkRbrain::Views::OrgansList<T>>("/api/organs");
            _server.add_resource<LinkRbrain::Views::Datasets<T>>("/api/datasets/(\\d+)");
            _server.add_resource<LinkRbrain::Views::DatasetsList<T>>("/api/datasets");
            _server.add_resource<LinkRbrain::Views::Groups<T>>("/api/datasets/(\\d+)/groups/(\\d+)");
            _server.add_resource<LinkRbrain::Views::GroupsList<T>>("/api/datasets/(\\d+)/groups");
            // queries
            _server.add_resource<LinkRbrain::Views::QueriesList<T>>("/api/queries");
            _server.add_resource<LinkRbrain::Views::Queries<T>>("/api/queries/(\\d+)");
            _server.add_resource<LinkRbrain::Views::QueriesPdf<T>>("/api/queries/(\\d+)/pdf");
            // _server.add_resource<LinkRbrain::Views::QueriesGroup<T>>("/api/queries/(\\d+)/groups/(\\d+)");
            // _server.add_resource<LinkRbrain::Views::QueriesGroupList<T>>("/api/queries/(\\d+)/groups");
            // _server.add_resource<LinkRbrain::Views::QueriesGroupSubgroup<T>>("/api/queries/(\\d+)/groups/(\\d+)/subgroups");
            // _server.add_resource<LinkRbrain::Views::QueriesGroupSubgroupList<T>>("/api/queries/(\\d+)/groups/(\\d+)/subgroups/(\\d+)");
            // users & identification
            _server.add_resource<LinkRbrain::Views::Users<T>>("/api/users/(\\d+)");
            _server.add_resource<LinkRbrain::Views::UsersMe<T>>("/api/users/me");
            _server.add_resource<LinkRbrain::Views::UsersList<T>>("/api/users");
            _server.add_resource<LinkRbrain::Views::TokensList<T>>("/api/tokens");
            // test point
            _server.add_resource<LinkRbrain::Views::Test<T>>("/api/test");
            // redirections
            _server.add_redirection("^/platform/?$", "/organs.html", Views::Redirection::Invisible, true);
            _server.add_redirection("^/platform/[\\w\\-]+$", "/platform.html", Views::Redirection::Invisible, true);
            _server.add_redirection("^/(\\w+)/?$", "/$1.html", Views::Redirection::Invisible, true);
            // rewrites
            _server.add_redirection("^/(.+\\.css)$", "/css/$1", Views::Redirection::Invisible, true);
            _server.add_redirection("^/(.+\\.js)$", "/js/$1", Views::Redirection::Invisible, true);
            _server.add_redirection("^/(.+\\.json)$", "/json/$1", Views::Redirection::Invisible, true);
            _server.add_redirection("^/(.+\\.obj)$", "/obj/$1", Views::Redirection::Invisible, true);
            _server.add_redirection("^/(.+\\.(?:jpe?g|png|gif|ico))$", "/images/$1", Views::Redirection::Invisible, true);
            _server.start();
        }

        Network::Server::HTTP::Server& get_server() {
            return _server;
        }

    protected:

        virtual const std::string get_logger_name() {
            return "LinkRbrain::HTTPController";
        }

    private:

        Network::Server::HTTP::Server _server;

    };

} // LinkRbrain::Controllers


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__HTTPCONTROLLER_HPP
