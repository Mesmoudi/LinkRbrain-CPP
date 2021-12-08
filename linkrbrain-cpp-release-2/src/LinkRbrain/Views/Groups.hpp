#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__GROUPS_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__GROUPS_HPP


#include "./BaseView.hpp"


namespace LinkRbrain::Views {


    template <typename T>
    class GroupsList : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef LinkRbrain::Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            // fetch dataset
            const size_t dataset_id = std::stoul(request.url_parameters[1]);
            const auto& dataset_controller = app.get_data_controller().get_dataset(dataset_id);
            // parse options
            const size_t offset = std::stoul(request.query.get("offset", "0"));
            const size_t limit = std::min(std::stoul(request.query.get("limit", "20")), 100UL);
            const std::string with_points_string = request.query.get("with_points", "false");
            const bool with_points = (with_points_string[0] == 't' || with_points_string[0] == '1');
            // retrieve identifiers
            std::unordered_set<uint64_t> identifiers;
            try {
                std::string string_identifiers = request.query.get("identifiers");
                std::istringstream iss_identifiers(string_identifiers);
                std::string identifier;
                while (std::getline(iss_identifiers, identifier, ',')) {
                    identifiers.insert(std::stoul(identifier));
                }
            } catch (const Exceptions::NotFoundException&) {}
            // retrieve pattern
            std::vector<std::string> keywords;
            try {
                std::string lowered_keywords = request.query.get("contains");
                std::transform(lowered_keywords.begin(), lowered_keywords.end(), lowered_keywords.begin(), tolower);
                std::istringstream iss_keywords(lowered_keywords);
                std::string keyword;
                while (std::getline(iss_keywords, keyword, ' ')) {
                    keywords.push_back(keyword);
                }
            } catch (const Exceptions::NotFoundException&) {}
            // fill response with data
            dataset_controller.serialize_groups(
                response.data,
                keywords,
                identifiers,
                offset,
                limit,
                with_points
            );
        }
    };


    template <typename T>
    class Groups : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef LinkRbrain::Controllers::AppController<T> AppController;

        virtual void GET(const Request& request, Response& response, AppController& app) {
            // fetch dataset
            const size_t dataset_id = std::stoul(request.url_parameters[1]);
            const auto& dataset_controller = app.get_data_controller().get_dataset(dataset_id);
            // make response
            const size_t group_id = std::stoul(request.url_parameters[2]);
            dataset_controller.serialize_group(response.data, group_id);
        }
    };


} // LinkRbrain::Views


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__GROUPS_HPP
