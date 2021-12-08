#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__PDF__QUERYDOCUMENT_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__PDF__QUERYDOCUMENT_HPP


#include "LinkRbrain/Models/Query.hpp"

#include "./SlicesCache.hpp"
#include "PDF/Document.hpp"
#include "Generators/NumberName.hpp"
#include "Generators/ColorName.hpp"
#include "Types/Point.hpp"
#include "Exceptions/GenericExceptions.hpp"

#include <filesystem>
#include <string>
#include <map>


namespace LinkRbrain::PDF {

    static SlicesCache slices_cache;

    const ::PDF::Color logo_color = {0.173, 0.286, 0.392};
    const ::PDF::Color line_color = {
        (1.f + logo_color.r) / (1.f + 1.f),
        (1.f + logo_color.g) / (1.f + 1.f),
        (1.f + logo_color.b) / (1.f + 1.f)
    };
    const ::PDF::Color link_color = {
        2.f * logo_color.r,
        2.f * logo_color.g,
        2.f * logo_color.b,
    };
    static const ::PDF::Style default_style = {
        .font_name = "Ubuntu-L",
        .font_size = 9,
        .line_height = 1.8,
        .paragraph_spacing = 0.5,
        .font_color = {0, 0, 0},
        .background_color = {1, 1, 1},
        .margin_top = 2.0,
        .margin_bottom = 2.0,
        .margin_left = 2.0,
        .margin_right = 2.0,
    };
    static const ::PDF::Style bold_style = {
        .font_name = "Ubuntu-M",
        .font_color = {.25, .25, .25}
    };
    static const ::PDF::Style header_style = {
        .font_name = "Roboto-Thin",
        .font_size = 24,
        .line_height = 2.5,
        .paragraph_spacing = .5,
        .font_color = {.4, .4, .4}
    };

    static const ::PDF::Style table_style = {
        .margin_left = 0.2,
        .font_size = 9,
        .line_height = 2,
    };
    static const ::PDF::Style table_header_style = table_style | bold_style | ::PDF::Style{
        .background_color = {.3,.3,.3},
        .font_color = {1,1,1}
    };
    static const ::PDF::Style table_even_style = table_style | ::PDF::Style{
        .background_color = {1,1,1},
        .font_color = {0,0,0}
    };
    static const ::PDF::Style table_odd_style = table_style | ::PDF::Style{
        .background_color = {.9,.9,.9},
        .font_color = {0,0,0}
    };

    class QueryDocument : public ::PDF::Document {
    public:

        QueryDocument(const LinkRbrain::Models::Query& query) :
            _section_index(0),
            _figure_index(0),
            _query(query),
            _default_style(default_style),
            _header_style(header_style),
            ::PDF::Document(default_style)
        {
            load_font("var/fonts/roboto/Roboto-Thin.ttf");
            load_font("var/fonts/ubuntu/Ubuntu-L.ttf");
            load_font("var/fonts/ubuntu/Ubuntu-B.ttf");
            load_font("var/fonts/ubuntu/Ubuntu-M.ttf");
        }

        // make header & footer

        virtual void on_after_new_page() {
            const ::PDF::Style& style = get_style();
            ::PDF::Page& page = get_current_page();
            float x;
            // store original position
            const float x0 = get_x();
            const float y0 = get_y();
            // add header
            const float header_logo_width = 3;
            set_x(page.get_width() - style.margin_left - .5*style.margin_right - header_logo_width);
            set_y(y0 - 1.5);
            start_link("https://www.linkrdata.fr");
            append_image("var/images/logo_header_resized.png", header_logo_width);
            stop_link();
            add_horizontal_line(page.get_height() - style.margin_top + 0.01);
            // add footer
            const float footer_logo_width = 0.75;
            const size_t page_index = get_current_page_index();
            add_horizontal_line(style.margin_bottom );
            set_x(x0 - style.margin_left * 0.5);
            set_y(page.get_height() - style.margin_bottom - 2.0 * footer_logo_width);
            append_image("var/images/logo_footer_resized.png", footer_logo_width);
            page.write_raw(
                style.margin_left * 0.5 + 1.5 * footer_logo_width, style.margin_bottom * 0.5,
                "Copyright © LinkRdata 2021. All Right Reserved.",
                default_style | ::PDF::Style{.font_color = logo_color}
            );
            page.write_raw(
                page.get_width() - style.margin_right * 0.5, style.margin_bottom * 0.5,
                std::to_string(page_index + 1), default_style | ::PDF::Style{.font_name="Ubuntu-B", .font_color=logo_color}
            );
            // go back to original position
            set_x(x0);
            set_y(y0);
        }

        // horizontal lines

        void add_horizontal_line(const ::PDF::Color color = line_color) {
            add_horizontal_line(get_current_page().get_y());
        }

        void add_horizontal_line(const float y, const ::PDF::Color color = line_color) {
            const float a = 0.45; // a=0: horizontal line goes from left border to right border; a=1: horizontal line goes from left margin to right margin
            const ::PDF::Style& style = get_style();
            add_horizontal_line(
                y,
                a * style.margin_left,
                get_current_page().get_width() - a * style.margin_right,
                color
            );
        }

        void add_horizontal_line(const float y, const float x1, const float x2, const ::PDF::Color color = line_color) {
            get_current_page().fill(
                x1, y,
                x2 - x1, 0.01,
                color);
        }

        // links

        void append_link(const std::string& text, const ::PDF::Link& link) {
            const ::PDF::Style style = get_style();
            set_style(style | ::PDF::Style({.font_color = link_color}));
            start_link(link);
            append_text(text);
            stop_link();
            set_style(style);
        }

        // titles

        void append_subtitle(const std::string& text, const bool with_toc = true) {
            return append_title(text, with_toc, true);
        }
        void append_title(const std::string& text, const bool with_toc = true, const bool is_subtitle = false) {
            // remember style
            const ::PDF::Style style = get_style();
            new_line();
            // write style where we are title style
            if (is_subtitle) {
                set_style(_header_style | ::PDF::Style{.font_size = 18});
            } else {
                set_style(_header_style);
            }
            append_text(text);
            new_paragraph();
            // add an element to the table of contents
            if (with_toc) {
                append_title_toc(text, get_current_page_index(), is_subtitle);
            }
            // go back to previous style
            set_style(style);
        }

        void append_title_toc(const std::string& text, const size_t linked_page_index, const bool is_subtitle = false) {
            const ::PDF::Style saved_style = get_style();
            const size_t current_page_index = get_current_page_index();
            //
            set_current_page_index(0);
            if (is_subtitle) {
                set_style(_header_style | ::PDF::Style{.font_size = 18, .line_height = 2.4});
            } else {
                set_style(_header_style | ::PDF::Style{.line_height = 1.8});
            }
            start_link(linked_page_index);
            if (is_subtitle) {
                set_x(get_x() + .5);
            }
            append_text(text);
            const float x1 = get_x();
            const float x2 = get_current_page().get_width() - get_style().margin_left - get_style().margin_right - 1.f;
            set_x(x2);
            append_text(std::to_string(linked_page_index + 1));
            stop_link();
            new_line();
            add_horizontal_line(
                get_current_page().get_height() - get_y() - get_style().margin_top + .02f,
                x1 + get_style().margin_left + .15f,
                x2 + get_style().margin_left - .10f,
                line_color
            );
            //
            set_style(saved_style);
            set_current_page_index(current_page_index);
        }

        // front section

        void add_front_section() {
            append_title("Foreword", false);
            // intro
            append_paragraph("Thank you for using LinkRbrain, the open access platform integrating knowledge produced by the scientific community. LinkRbrain is part of LinkRdata, a host for open data software.");
            append_paragraph("Do not forget to cite our article if you are using the results of this document in your publication:");
            // citation formats
            Types::Table table;
            table.add_row("AMA", "Mesmoudi S, Rodic M, Cioli C, Cointet JP, Yarkoni T, Burnod Y. LinkRbrain: multi-scale data integrator of the brain. J Neurosci Methods. 2015;241:44-52. doi:10.1016/j.jneumeth.2014.12.008");
            table.add_row("MLA", "Mesmoudi, Salma et al. “LinkRbrain: multi-scale data integrator of the brain.” Journal of neuroscience methods vol. 241 (2015): 44-52. doi:10.1016/j.jneumeth.2014.12.008");
            table.add_row("APA", "Mesmoudi, S., Rodic, M., Cioli, C., Cointet, J. P., Yarkoni, T., & Burnod, Y. (2015). LinkRbrain: multi-scale data integrator of the brain. Journal of neuroscience methods, 241, 44–52. https://doi.org/10.1016/j.jneumeth.2014.12.008");
            table.add_row("NLM", "Mesmoudi S, Rodic M, Cioli C, Cointet JP, Yarkoni T, Burnod Y. LinkRbrain: multi-scale data integrator of the brain. J Neurosci Methods. 2015 Feb 15;241:44-52. doi: 10.1016/j.jneumeth.2014.12.008. Epub 2014 Dec 18. PMID: 25528112; PMCID: PMC4418971.");
            append_table(table, {2}, {}, table_odd_style, table_even_style);
            // more intro
            append_paragraph("You can inform us of any enquiry at contact@linkrdata.fr, and we will come back to you as soon as possible (usually in less than a week). If you need the expertise of our team to help you understand better the results of this document, do not hesitate to contact us. We can also give you more information about additional services of LinkRbrain, like the cross-references by adding your specific patient data on the platform or the integration of your experimental or bibliographic data for example. Feel free to add in your message any feedback about your experience on LinkRbrain, it will help us understand your needs better, and therefore improve the platform according to your usage.");
            append_paragraph("Disclaimer: the results presented on this document are only indicative, the site owner can not be held responsible for non accuracy of the data or the results presented.");
            // table of contents
            add_horizontal_line();
            append_title_toc("Foreword", 0);
        }

        // view section

        void add_view_section(const std::map<std::string, std::filesystem::path>& figures_2D, const Types::Point<float>& origin_2D, const std::vector<std::pair<std::string, std::filesystem::path>>& figures_3D) {
            // this is the beginning!
            new_page();
            append_title("Representations");
            // represent color legend
            append_text("In all the figures below, ");
            const auto& groups_data = _query.groups.get_vector();
            for (size_t i = 0, n = groups_data.size(); i < n; i++) {
                const auto& group_data = groups_data[i];
                const std::string label = group_data.get("label").get_string();
                const float hue = group_data.get("hue").get_number();
                const Types::Color color = Types::Color::from_hsl(hue, 1.f, 0.4f);
                set_style(bold_style | ::PDF::Style{.font_color = color});
                append_text(label);
                set_style(_default_style);
                append_text(" is in " + Generators::ColorName::get_english_name(color));
                if (i == n - 2) {
                    append_text(" and ");
                } else if (i == n - 1) {
                    append_text(".");
                } else {
                    append_text(", ");
                }
            }
            // add real views bodies
            if (figures_2D.size()) {
                add_2d_view_section(figures_2D, origin_2D);
            }
            if (figures_3D.size()) {
                const int figure_count_on_first_page = figures_2D.size() ? 2 : 4;
                add_3d_view_section(figures_3D, figure_count_on_first_page);
            }
        }

        void add_2d_view_section_figure(const std::filesystem::path& figure, const float figure_width, const std::string& slice_name, const float position, const std::string& plane_label) {
            const std::string& png_buffer = slices_cache.get_cached_png_slice(slice_name, position);
            HPDF_Image hpdf_image = HPDF_LoadPngImageFromMem(_hpdf_doc, (HPDF_BYTE*) png_buffer.data(), png_buffer.size());
            const std::string text = plane_label + " plane, " + slice_name.back() + " = " + round_coordinate(position);
            add_figure(hpdf_image, figure_width, text);
            const float y0 = get_y();
            append_image(figure, figure_width);
            set_y(y0);
        }
        void add_2d_view_section(const std::map<std::string, std::filesystem::path>& figures, const Types::Point<float>& origin) {
            // SEE https://www.researchgate.net/figure/Representation-en-coupe-du-cerveau-a-Plans-de-coupe-conventionnels-de-neuroanatomie-et_fig3_278643302
            append_subtitle("2D views");
            // configuration
            const float figures_spacing = 0.2f;
            const std::vector<std::pair<std::string, std::string>> planes = {
                {"yzx", "Sagittal"},
                {"xzy", "Coronal"},
                {"xyz", "Axial"},
            };
            // determine figures sizes
            const float available_width = get_current_page().get_width() - get_style().margin_left - get_style().margin_right - (figures.size() - 1) * figures_spacing;
            std::vector<std::pair<float, float>> figures_sizes;
            float figures_widths_sum = 0.f;
            float figures_height_max = 0.f;
            for (const auto& [slice_name, plane_label] : planes) {
                const float width = slices_cache.get_slice_size(slice_name[0]);
                const float height = slices_cache.get_slice_size(slice_name[1]);
                figures_sizes.push_back({width, height});
                figures_widths_sum += width;
                if (height > figures_height_max) {
                    figures_height_max = height;
                }
            }
            // rescale sizes
            const float scale = available_width / figures_widths_sum;
            for (auto& size : figures_sizes) {
                size.first *= scale;
                size.second *= scale;
            }
            figures_height_max *= scale;
            // render figures to PDF
            const float y0 = get_y();
            int i = 0;
            for (const auto& [slice_name, plane_label] : planes) {
                const float figure_width = figures_sizes[i].first;
                const float figure_height = figures_sizes[i].second;
                set_y(y0 + figures_height_max - figure_height);
                const float position = origin.values[slice_name[2] - 'x'];
                //
                const auto& it = figures.find(slice_name);
                if (it == figures.end()) {
                    throw Exceptions::NotFoundException("Could not find 2D slice: " + slice_name);
                }
                const std::filesystem::path& figure = it->second;
                //
                add_2d_view_section_figure(figure, figure_width, slice_name, position, plane_label);
                increment_x(figure_width + figures_spacing);
                ++i;
            }
            increment_y(figures_height_max);
            new_line();
            new_paragraph();
            return;
        }

        void add_3d_view_section(const std::vector<std::pair<std::string, std::filesystem::path>>& figures, const int figure_count_on_first_page) {
            // represent 3D figures
            append_subtitle("3D views");
            for (size_t i = 0, n = figures.size(); i < n; i++) {
                const auto& figure = figures[i];
                const float y0 = get_y();
                const float image_width = 8;
                const float xm = (get_current_page().get_width() - 2 * image_width - get_style().margin_left - get_style().margin_right) / 2.f + (get_style().margin_left + image_width);
                const float y = add_figure(figure.second, image_width, figure.first);
                if (i + 1 == figure_count_on_first_page) {
                    new_page();
                } else if (i % 2 == 0) {
                    set_x(xm);
                    set_y(y0);
                } else {
                    set_x(0);
                    set_y(y + 1);
                }
            }
        }

        // graph section

        void add_graph_section(const std::filesystem::path& figure_path) {
            const std::string dataset_label = _query.settings.get("correlations").get("dataset").get("label").get_string();
            new_page();
            append_title("Graph");
            //
            append_text("The graph show how strongly ");
            const auto& groups_data = _query.groups.get_vector();
            for (size_t i = 0, n = groups_data.size(); i < n; i++) {
                const auto& group_data = groups_data[i];
                const std::string label = group_data.get("label").get_string();
                const float hue = group_data.get("hue").get_number();
                const Types::Color color = Types::Color::from_hsl(hue, 1.f, 0.4f);
                set_style(bold_style | ::PDF::Style{.font_color = color});
                append_text(label);
                set_style(_default_style);
                append_text(", ");
            }
            append_text(" and groups from the ");
            set_style(bold_style | ::PDF::Style{.font_color = {.5,.5,.5}});
            append_text(dataset_label + " dataset");
            set_style(_default_style);
            append_text(" are correlated with each other.");
            //
            new_paragraph();
            append_text("A link can be:");
            for (const auto& group_data : groups_data) {
                const float hue = group_data.get("hue").get_number();
                const Types::Color color = Types::Color::from_hsl(hue, 1.f, 0.4f);
                const ::PDF::Style legend_style = bold_style | ::PDF::Style{.font_color = color};
                new_line();
                append_text("  -  ");
                set_style(legend_style);
                append_text(Generators::ColorName::get_english_name(color));
                set_style(_default_style);
                append_text(" if it connects ");
                set_style(legend_style);
                append_text(group_data.get("label").get_string());
                set_style(_default_style);
                append_text(" with groups of the ");
                set_style(bold_style | ::PDF::Style{.font_color = {.5,.5,.5}});
                append_text(dataset_label + " dataset");
                set_style(_default_style);
            }
            new_line();
            append_text("  -  ");
            set_style(bold_style | ::PDF::Style{.font_color = {.5,.5,.5}});
            append_text("gray");
            set_style(_default_style);
            append_text(" if it connects groups of the ");
            set_style(bold_style | ::PDF::Style{.font_color = {.5,.5,.5}});
            append_text(dataset_label + " dataset");
            set_style(_default_style);
            append_text(" with one another");
            new_line();
            //
            new_paragraph();
            new_line();
            add_figure(figure_path, 12.f, "Graph of correlations between selected groups and cognitive tasks");
        }

        // correlations section

        void add_correlations_section() {
            new_page();
            add_correlations_section_intro();
            add_correlations_section_table();
        }

        void add_correlations_section_intro() {
            // title depends on correlated dataset
            const std::string dataset_label = _query.settings.get("correlations").get("dataset").get("label").get_string();
            append_title("Correlations with " + dataset_label);
            // first paragraph: contextualization
            append_text("We based the correlations calculations on the overlap between the weighted points given in the user-defined groups ");
            const auto& groups_data = _query.groups.get_vector();
            for (size_t i=0, n=groups_data.size(); i<n; ++i) {
                const auto& group_data = groups_data[i];
                const std::string label = group_data.get("label").get_string();
                const float hue = group_data.get("hue").get_number();
                const Types::Color color = Types::Color::from_hsl(hue, 1.f, 0.4f);
                set_style(bold_style | ::PDF::Style{.font_color = color});
                append_text(label);
                set_style(_default_style);
                if (i == n - 2) {
                    append_text(" and ");
                } else {
                    append_text(", ");
                }
            }
            append_text(" and groups from the ");
            set_style(bold_style | ::PDF::Style{.font_color = {.2,.2,.2}});
            append_text(dataset_label);
            set_style(_default_style);
            append_text(" dataset.");
            // second paragraph: explanation from context
            new_paragraph();
            append_text("How are scores calculated? Let us designate respectively with the letters ");
            set_style(bold_style);
            append_text("A");
            set_style(_default_style);
            append_text(" and ");
            set_style(bold_style);
            append_text("B");
            set_style(_default_style);
            append_text(" the groups ");
            float max_score = 0.f;
            size_t max_score_group_index = 0;
            const auto& scores = _query.correlations.get(0).get("scores").get_vector();
            for (size_t i = 1, n = scores.size(); i < n; i++) {
                const double score = scores[i].get_number();
                if (score > max_score) {
                    max_score = score;
                    max_score_group_index = i - 1;
                }
            }
            const std::string& max_score_label = _query.groups.get(max_score_group_index).get("label").get_string();
            set_style(bold_style);
            append_text(max_score_label);
            set_style(_default_style);
            append_text(" and ");
            set_style(bold_style);
            append_text(_query.correlations.get(0).get("label").get_string());
            set_style(_default_style);
            append_text(" from the dataset " + dataset_label + ". ");
            // general explanations: first paragraph
            append_text("These groups can be represented as sets of weighted points:");
            new_paragraph();
            new_line();
            append_image("var/images/formula_groups.png", 4.2);
            new_line();
            append_text("where ");
            {
                auto [x0, y0] = get_xy();
                increment_y(0.22);
                append_image("var/images/formula_mu_i.png", .25);
                set_xy(x0 + .25, y0);
            }
            append_text(" and ");
            {
                auto [x0, y0] = get_xy();
                increment_y(0.22);
                append_image("var/images/formula_nu_j.png", .25);
                set_xy(x0 + .25, y0);
            }
            append_text(" are the respective weights of the points ");
            {
                auto [x0, y0] = get_xy();
                increment_y(0.15);
                append_image("var/images/formula_m_i.png", .4);
                set_xy(x0 + .4, y0);
            }
            append_text(" and ");
            {
                auto [x0, y0] = get_xy();
                increment_y(0.15);
                append_image("var/images/formula_n_j.png", .4);
                set_xy(x0 + .4, y0);
            }
            append_text(". These weights can express the intensity of either genetic overexpression, or activation mesured by fMRI");
            //
            new_paragraph();
            append_text("We use the following formula to compute the raw correlation score ");
            {
                auto [x0, y0] = get_xy();
                increment_y(0.23);
                append_image("var/images/formula_sigma.png", .18);
                set_xy(x0 + .18, y0);
            }
            append_text(":");
            new_paragraph();
            new_line();
            append_image("var/images/formula_sigma_value.png", 6.7);
            new_line();
            //
            append_text("where r is the reference radius (r = 10mm, according to the meta-analysis), ");
            append_text("and d is the euclidian distance between two points.");
            new_paragraph();
            append_text("The normalized score between the groups of weighted points A and B is finally computed with the formula:");
            new_paragraph();
            new_line();
            append_image("var/images/formula_score.png", 4.1);
            new_line();
            append_text("For readability purposes, this result is displayed as multiplied by 100, then rounded to the nearest integer.");
            new_paragraph();
        }

        void add_correlations_section_table() {
            // prepare to generate page
            const auto& correlations = _query.correlations.get_vector();
            if (correlations.size() == 0) {
                append_paragraph("The given groups do not correlate with any of the sets present in our database.");
                return;
            }
            const size_t groups_count = _query.groups.get_vector().size();
            // prepare table
            const std::string& dataset_label = _query.settings["correlations"]["dataset"]["label"].get_string();
            std::vector<std::string> header_row = {"Label of " + dataset_label.substr(0, dataset_label.size() - 1)};
            if (groups_count > 1) {
                header_row.push_back("Overall correlation score");
            }
            for (size_t i = 0; i < groups_count; i++) {
                header_row.push_back("Correlation score with " + _query.groups[i]["label"].get_string());
            }
            Types::Table table(header_row);
            // add table content
            for (const Types::Variant& correlation : correlations) {
                std::vector<std::string> row;
                row.push_back(correlation.get("label").get_string());
                const auto& scores = correlation["scores"].get_vector();
                for (size_t i = 0; i < groups_count; i++) {
                    row.push_back(round_coordinate(100.0 * scores[i].get_number()));
                }
                if (groups_count > 1) {
                    row.push_back(round_coordinate(100.0 * scores[groups_count].get_number()));
                }
                table.add_row(row);
            }
            // add table to pdf
            new_paragraph();
            append_table(table, {}, table_header_style | ::PDF::Style{
                .line_height = 1.8,
                .margin_top = .1,
                .margin_bottom = .1,
            }, table_odd_style, table_even_style);
        }

        // groups section

        void add_subgroups_info(const Types::Variant& subgroups) {

        }

        void add_group_subsection(const Types::Variant& group_data) {
            // subsection title
            append_subtitle(group_data.get("label").get_string());
            // introduction
            const auto& subgroups = group_data.get("subgroups").get_vector();
            if (subgroups.size() == 0) {
                append_text("This group does not contain any data.");
                return;
            } else {
                append_text("This group contains ");
                set_style(bold_style);
                append_text(std::to_string(group_data.get("points").get_vector().size()));
                set_style(default_style);
                append_text(" points");
            }
            // rearrange data
            std::map<std::string, std::vector<std::string>> subgroups_by_type;
            for (const auto& subgroup : subgroups) {
                std::string type = subgroup.get("source").get_string();
                std::string label;
                //
                if (type == "datasets") {
                    type = subgroup.get("metadata").get("dataset").get("label").get_string();
                    label = subgroup.get("metadata").get("group").get("label").get_string();
                } else if (type == "text_file" || type == "nifti_file") {
                    type = "file";
                    label = subgroup.get("metadata").get("original_path").get_string();
                } else if (type == "text") {
                    // no label to consider
                }
                subgroups_by_type[type].push_back(label);
            }
            // represent data
            append_text(".");
            size_t type_index = 0;
            for (const auto& [type, labels] : subgroups_by_type) {
                // intruducing the intro
                new_paragraph();
                if (subgroups_by_type.size() == 1) {
                    append_text("These");
                } else {
                    static const std::string beginnings[] = {"Some of these", "Other", "Additionally,", "On top of the previously explicited sources, some", "Finally, the rest of the"};
                    append_text(beginnings[type_index]);
                    type_index = (type_index + 1) % subgroups_by_type.size();
                }
                append_text(" points ");
                // the introduction depends on the source type
                const std::string suffix = (labels.size() > 1) ? "s" : "";
                if (type.compare(0, 9, "functions") == 0) {
                    append_text("are activation peaks corresponding to the function" + suffix);
                } else if (type.compare(0, 5, "genes") == 0) {
                    append_text("are zones where the gene" + suffix);
                } else if (type == "text") {
                    append_text("have been parsed from text provided in the web browser");
                } else if (type == "file") {
                    append_text("were extracted from the user-provided file" + suffix);
                } else {
                    append_text("the " + type + " dataset group" + suffix);
                }
                // the rest is easier to represent
                if (labels.size() == 1) {
                    append_text(" \"");
                    set_style(bold_style);
                    append_text(labels[0]);
                    set_style(default_style);
                    append_text("\".");
                } else {
                    append_text(":");
                    new_paragraph();
                    bool is_first = true;
                    for (const auto& label : labels) {
                        if (is_first) is_first = false;
                        else new_line();
                        append_text("  -  ");
                        set_style(bold_style);
                        append_text(label);
                        set_style(default_style);
                    }
                }
                // the conclusion also depends on the source type
                if (type.compare(0, 9, "functions") == 0) {
                    new_paragraph();
                    if (labels.size() == 1) {
                        append_text("The activation peaks associated with this function have been reconstructed from the ones published in the publications parsed with ");
                    } else {
                        append_text("The activation peaks associated with these functions have been reconstructed from the ones published in the publications parsed with ");
                    }
                    append_link("ACE", "https://github.com/neurosynth/ACE/tree/master/ace");
                    append_text(" and ");
                    append_link("CorTexT", "https://www.cortext.net/");
                    append_text(".");
                } else if (type.compare(0, 5, "genes") == 0) {
                    if (labels.size() == 1) {
                        append_text(" is overexpressed.");
                    } else {
                        new_paragraph();
                        append_text("are overexpressed.");
                    }
                    append_text(" The localization of genetic overexpression has been extracted from the ");
                    append_link("Allen Foundation human brain atlas", "https://help.brain-map.org/display/humanbrain/Documentation");
                    append_text(".");
                }
            }
            // now, show points in table
            Types::Table table({"x", "y", "z", "weight"});
            for (const Types::Variant& point_data : group_data.get("points").get_vector()) {
                table.add_row(
                    round_coordinate(point_data[0].get_number(), 1),
                    round_coordinate(point_data[1].get_number(), 1),
                    round_coordinate(point_data[2].get_number(), 1),
                    round_coordinate(point_data[3].get_number(), 3)
                );
            }
            new_paragraph();
            append_table(table, {2.5, 2.5, 2.5, 2.5}, table_header_style, table_odd_style, table_even_style);
        }

        void add_groups_section() {
            // main section
            const size_t groups_count = _query.groups.get_vector().size();
            switch (groups_count) {
                case 0:
                    return;
                case 1:
                    new_page();
                    append_title("Group");
                    append_text("The query consists of one group: ");
                    break;
                default:
                    new_page();
                    append_title("Groups");
                    append_text("The query consists of " + Generators::NumberName::get_english_name(groups_count) + " groups: ");
                    break;
            }
            for (size_t group_index = 0; group_index < groups_count; group_index++) {
                set_style(bold_style);
                append_text(_query.groups.get(group_index).get("label").get_string());
                set_style(default_style);
                switch (groups_count - group_index) {
                    case 1:
                        append_text(".");
                        break;
                    case 2:
                        append_text(" and ");
                        break;
                    default:
                        append_text(", ");
                        break;
                }
            }
            // one subsection per group
            for (const auto& group_data : _query.groups.get_vector()) {
                add_group_subsection(group_data);
            }
        }

    private:

        const std::string round_coordinate(const float coordinate, const char decimals = 0) {
            char format[] = "%.0f";
            if (decimals > 0 && decimals < 10) {
                format[2] = '0' + decimals;
            }
            char buffer[32];
            const int size = snprintf(buffer, sizeof(buffer), format, coordinate);
            if (size < 0) {
                return {};
            }
            return {buffer, size};
        }

        const float add_figure(const std::filesystem::path& path, const float width, const std::string legend = "") {
            const HPDF_Image image = load_image(path, width);
            return add_figure(image, width, legend);
        }
        const float add_figure(const HPDF_Image& image, const float width, const std::string legend = "") {
            // save original position
            const float x0 = get_x();
            const float y0 = get_y();
            // append image
            const float height = append_image(image, width);
            // write legend
            increment_y(0.25);
            append_text("Fig. " + std::to_string(++_figure_index) + (legend.size() ? " - " : ""), bold_style);
            append_text(legend);
            // fetch theoretical end
            new_paragraph();
            const float y1 = get_y();
            // go back to original position
            set_x(x0);
            set_y(y0);
            // return theoretical end
            return y1;
        }

        //
        size_t _section_index;
        size_t _figure_index;
        const LinkRbrain::Models::Query& _query;
        //
        ::PDF::Style _default_style;
        ::PDF::Style _header_style;

    };

} // LinkRbrain::Commands

#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__PDF__QUERYDOCUMENT_HPP
