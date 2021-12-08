#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__QUERIES_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__QUERIES_HPP


#include "./BaseView.hpp"


namespace LinkRbrain::Views {


    template <typename T>
    class Queries : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            const size_t query_id = std::stoul(request.url_parameters[1]);
            auto query = app.get_db_controller().queries.fetch(query_id);
            app.get_db_controller().queries.serialize(response.data, query);
        }

        virtual void DELETE(const Request& request, Response& response, AppController& app) {
            const size_t query_id = std::stoul(request.url_parameters[1]);
            auto query = app.get_db_controller().queries.fetch(query_id);
            app.get_db_controller().queries.remove(query);
            app.get_db_controller().queries.serialize(response.data, query);
        }

        virtual void PATCH(const Request& request, Response& response, AppController& app) {
            // retrieve request info
            const size_t query_id = std::stoul(request.url_parameters[1]);
            auto query = app.get_db_controller().queries.fetch(query_id);
            auto data = request.data;
            // those things should not be modified by the user
            if (data.has("correlations") || data.has("graph")) {
                const std::string key = data.has("correlations") ? "correlations" : "graph";
                throw Exceptions::BadDataException("Do not try to set correlations or graph manually. This is a job for the back-end.", {
                    {"resource", "queries"},
                    {"action", "update"},
                    {"field", key},
                    {"problem", "readonly"}
                });
            }
            // computation gets invalidated by some query changes
            if (data.has("groups") || (data.has("settings") && data["settings"]["correlations"]["dataset"]["id"] != query.settings["correlations"]["dataset"]["id"])) {
                data["is_computed"] = false;
                data["correlations"].unset();
                data["graph"].unset();
            }
            // save and return
            app.get_db_controller().queries.update_data(query, data);
            // recompute when specified
            if (data.has("is_computed") && data["is_computed"].get_boolean()) {
                auto& dataset_controller = app.get_data_controller().get_dataset(query.settings["correlations"]["dataset"]["id"]);
                dataset_controller.compute(query);
                app.get_db_controller().queries.update(query, {"correlations", "graph"});
            }
            // serialize
            app.get_db_controller().queries.serialize(response.data, query);
        }

    };


    template <typename T>
    class QueriesList : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            response.data = {
                {"data", Types::Variant::Vector}
            };
            auto& data = response.data["data"].get_vector();
            for (auto& query : app.get_db_controller().queries.fetch_all()) {
                data.push_back(Types::Variant::Map);
                app.get_db_controller().queries.serialize(data.back(), query);
            }
        }

        virtual void POST(const Request& request, Response& response, AppController& app) {
            if (request.data.has("correlations") || request.data.has("graph")) {
                const std::string key = request.data.has("correlations") ? "correlations" : "graph";
                throw Exceptions::BadDataException("Do not try to set correlations or graph manually. This is a job for the back-end.", {
                    {"resource", "queries"},
                    {"action", "insert"},
                    {"field", key},
                    {"problem", "readonly"}
                });
            }
            response.code = 201;
            auto query = app.get_db_controller().queries.insert_data(request.data);
            app.get_db_controller().queries.serialize(response.data, query);
        }

    };


} // LinkRbrain::Views


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__QUERIES_HPP
