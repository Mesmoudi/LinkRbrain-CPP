#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__USERS_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__USERS_HPP


#include "./BaseView.hpp"


namespace LinkRbrain::Views {


    template <typename T>
    class Users : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            const size_t user_id = std::stoul(request.url_parameters[1]);
            auto user = app.get_db_controller().users.fetch(user_id);
            app.get_db_controller().users.serialize(response.data, user);
        }

        virtual void DELETE(const Request& request, Response& response, AppController& app) {
            const size_t user_id = std::stoul(request.url_parameters[1]);
            auto user = app.get_db_controller().users.fetch(user_id);
            app.get_db_controller().users.remove(user);
            app.get_db_controller().users.serialize(response.data, user);
        }

        virtual void PATCH(const Request& request, Response& response, AppController& app) {
            const size_t user_id = std::stoul(request.url_parameters[1]);
            auto user = app.get_db_controller().users.fetch(user_id);
            app.get_db_controller().users.update_data(user, request.data);
            app.get_db_controller().users.serialize(response.data, user);
        }

    };


    template <typename T>
    class UsersList : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            response.data = {
                {"data", Types::Variant::Vector}
            };
            auto& data = response.data["data"].get_vector();
            for (auto& user : app.get_db_controller().users.fetch_all()) {
                data.push_back(Types::Variant::Map);
                app.get_db_controller().users.serialize(data.back(), user);
            }
        }

        virtual void POST(const Request& request, Response& response, AppController& app) {
            response.code = 201;
            auto user = app.get_db_controller().users.insert_data(request.data);
            app.get_db_controller().users.serialize(response.data, user);
        }

    };


    template <typename T>
    class UsersMe : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            const Models::User& user = app.get_tokens_controller().get_user(request);
            app.get_db_controller().users.serialize(response.data, user);
        }
    };

} // LinkRbrain::Views


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__USERS_HPP
