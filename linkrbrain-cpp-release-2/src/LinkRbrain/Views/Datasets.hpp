#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__DATASETS_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__DATASETS_HPP


#include "./BaseView.hpp"


namespace LinkRbrain::Views {


    template <typename T>
    class DatasetsList : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef LinkRbrain::Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            response.data["data"].set_vector();
            std::vector<Types::Variant>& data = response.data["data"].get_vector();
            const auto& datasets = app.get_data_controller().get_datasets();
            const size_t n = datasets.size();
            data.resize(n);
            for (size_t i=0; i<n; ++i) {
                datasets[i]->serialize(data[i], false);
            }
        }
    };


    template <typename T>
    class Datasets : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef LinkRbrain::Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            const size_t dataset_id = std::stoul(request.url_parameters[1]);
            const auto& dataset_controller = app.get_data_controller().get_dataset(dataset_id);
            dataset_controller.serialize(response.data, true);
        }
    };


} // LinkRbrain::Views


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__DATASETS_HPP
