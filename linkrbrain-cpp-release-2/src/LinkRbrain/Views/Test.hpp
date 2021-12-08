#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__TEST_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__TEST_HPP


#include "./BaseView.hpp"


namespace LinkRbrain::Views {


    template <typename T>
    class Test : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            try {
                app.get_db_controller().users.serialize(response.data["user"], app.get_tokens_controller().get_user(request));
            } catch (const Exceptions::UnauthorizedException&) {}
            response.data["request"] = {
                {"ip", request.ip},
                {"headers", Types::Variant(request.headers)}
            };
        }
        virtual void POST(const Request& request, Response& response, AppController& app) {
            // retrieve user data
            try {
                app.get_db_controller().users.serialize(response.data["user"], app.get_tokens_controller().get_user(request));
            } catch (const Exceptions::UnauthorizedException&) {}
            // retrieve uploaded files
            response.data["files"].set_vector();
            for (const auto& [key, file] : request.files.get_all()) {
                response.data["files"].push_back({
                    {"key", file.key},
                    {"filename", file.filename},
                    {"content_type", file.content_type},
                    {"transfer_encoding", file.transfer_encoding},
                    {"size", file.data.size()}
                });
            }
            // retrieve other request data
            response.data["request"] = {
                {"ip", request.ip},
                {"headers", Types::Variant(request.headers)},
                {"data", request.data}
            };
        }

    };


} // LinkRbrain::Views


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__TEST_HPP
