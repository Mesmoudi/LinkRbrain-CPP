#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__TOKENS_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__TOKENS_HPP


#include "./BaseView.hpp"


namespace LinkRbrain::Views {


    template <typename T>
    class TokensList : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            // extract query parameters when available
            std::string username, password;
            try {
                username = request.query.get("username");
            } catch (const Exceptions::NotFoundException&) {
                throw Exceptions::BadDataException("Missing parameter: username", {
                    {"resource", "tokens"},
                    {"field", "username"}
                });
            }
            try {
                password = request.query.get("password");
            } catch (const Exceptions::NotFoundException&) {
                throw Exceptions::BadDataException("Missing parameter: password", {
                    {"resource", "tokens"},
                    {"field", "password"}
                });
            }
            // respond
            const Models::User user = app.get_db_controller().users.fetch_by_credentials(username, password);
            app.get_db_controller().users.serialize(response.data["user"], user);
            response.data["token"] = app.get_tokens_controller().get_token(user);
        }

    };


} // LinkRbrain::Views


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__TOKENS_HPP
