#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__QUERIESPDF_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__QUERIESPDF_HPP


#include "Types/DateTime.hpp"

#include "./BaseView.hpp"
#include "LinkRbrain/PDF/QueryDocument.hpp"


namespace LinkRbrain::Views {


    template <typename T>
    class QueriesPdf : public BaseView<T> {
    public:

        using BaseView<T>::BaseView;
        typedef Controllers::AppController<T> AppController;

        virtual void POST(const Request& request, Response& response, AppController& app) {
            // retrieve query
            const size_t query_id = std::stoul(request.url_parameters[1]);
            auto query = app.get_db_controller().queries.fetch(query_id);
            // reformat figures path
            // generate PDF: introduction
            LinkRbrain::PDF::QueryDocument document(query);
            document.add_front_section();
            // generate PDF: views
            const bool has_view2D = request.data.get("view2D").get_boolean();
            const bool has_view3D = request.data.get("view3D").get_boolean();
            if (has_view2D || has_view3D) {
                // 2D
                std::filesystem::path base_figures_path = "tmp/uploads";
                std::map<std::string, std::filesystem::path> view2D_figures;
                Types::Point<float> view2D_origin;
                if (has_view2D) {
                    for (const auto& view_figure : request.data.get("view2D_figures").get_vector()) {
                        view2D_figures.insert({
                            view_figure.get("name").get_string(),
                            base_figures_path / view_figure.get("path").get_string()
                        });
                    }
                }
                // 3D
                std::vector<std::pair<std::string, std::filesystem::path>> view3D_figures;
                for (const auto& view_figure : request.data.get("view3D_figures").get_vector()) {
                    view3D_figures.push_back({
                        view_figure.get("name").get_string(),
                        base_figures_path / view_figure.get("path").get_string()
                    });
                }
                // go!
                document.add_view_section(view2D_figures, view2D_origin, view3D_figures);
            }
            // generate PDF: graph
            if (request.data.get("graph").get_boolean()) {
                std::filesystem::path graph_figure_path = "tmp/uploads";
                graph_figure_path /= request.data.get("graph_figure").get_string();
                document.add_graph_section(graph_figure_path);
            }
            // generate PDF: correlations
            if (request.data.get("correlations").get_boolean()) {
                document.add_correlations_section();
            }
            // generate PDF: groups
            if (request.data.get("groups").get_boolean()) {
                document.add_groups_section();
            }
            // generate PDF: save file
            const std::filesystem::path date_path = Types::DateTime::now().get_date_string();
            const std::filesystem::path filename = "query_" + std::to_string(query_id) + "_" + std::to_string(Types::DateTime::now().get_timestamp()) + ".pdf";
            const std::filesystem::path url = "/" / date_path / filename;
            const std::filesystem::path base_pdf_path = "tmp/pdf" / date_path;
            const std::filesystem::path pdf_path = base_pdf_path / filename;
            std::filesystem::create_directories(base_pdf_path);
            document.save(pdf_path);
            // build response
            response.code = 201;
            response.data = {{"url", url}};
        }

    };


} // LinkRbrain::Views


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__VIEWS__QUERIESPDF_HPP
