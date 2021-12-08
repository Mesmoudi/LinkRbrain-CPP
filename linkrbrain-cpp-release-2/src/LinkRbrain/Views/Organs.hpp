#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__ORGANS_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__ORGANS_HPP


#include "./BaseView.hpp"


namespace LinkRbrain::Views {


    template <typename T>
    class OrgansList : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef LinkRbrain::Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            response.data["data"].set_vector();
            std::vector<Types::Variant>& data = response.data["data"].get_vector();
            const auto& organs = app.get_data_controller().get_organs();
            const size_t n = organs.size();
            data.resize(n);
            for (size_t i=0; i<n; ++i) {
                organs[i]->serialize(data[i], true);
            }
        }
    };


    template <typename T>
    class Organs : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef LinkRbrain::Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            const size_t organ_id = std::stoul(request.url_parameters[1]);
            const auto& organ = app.get_data_controller().get_organ(organ_id);
            organ.serialize(response.data, true);
        }
    };


} // LinkRbrain::Views


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__ORGANS_HPP
