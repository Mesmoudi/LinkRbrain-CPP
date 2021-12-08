#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__PDF_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__PDF_HPP


#include "CLI/Arguments/Command.hpp"

#include "LinkRbrain/PDF/QueryDocument.hpp"

#include "./linkrbrain.hpp"


namespace LinkRbrain::Commands {

    void linkrbrain_pdf(const CLI::Arguments::CommandResult& options) {
        _init_db_controller(options.get_parent());
        // fetch query from database
        const size_t query_id = std::stoul(options.get("query-id"));
        const LinkRbrain::Models::Query query = _get_db_controller().queries.fetch(query_id);
        // summarize into a nice PDF file
        LinkRbrain::PDF::QueryDocument document(query);
        document.add_front_section();
        document.add_view_section(
            {
                {"xyz", "tmp/uploads/2021-04-28/1619647997.932773_pdf-2D-0-xyz.svg"},
                {"yzx", "tmp/uploads/2021-04-28/1619647997.933231_pdf-2D-1-yzx.svg"},
                {"xzy", "tmp/uploads/2021-04-28/1619647997.933612_pdf-2D-2-xzy.svg"}
            },
            {0, 2, -20},
            {
                {"lateral-left", "tmp/uploads/2021-04-23/1619194461.616614_pdf-3D-93-0.png"},
                {"lateral-right", "tmp/uploads/2021-04-23/1619194462.423570_pdf-3D-93-1.png"},
                {"rostral", "tmp/uploads/2021-04-23/1619194463.007445_pdf-3D-93-2.png"},
                {"caudal", "tmp/uploads/2021-04-23/1619194463.619655_pdf-3D-93-3.png"},
                {"dorsal", "tmp/uploads/2021-04-23/1619194464.237252_pdf-3D-93-4.png"},
                {"ventral", "tmp/uploads/2021-04-23/1619194464.854924_pdf-3D-93-5.png"},
            }
        );
        document.add_graph_section("tmp/uploads/2021-04-23/1619194466.287078_pdf-graph-93.svg");
        document.add_correlations_section();
        document.add_groups_section();
        document.save(options.get("output-file"));
    } // linkrbrain_pdf

} // LinkRbrain::Commands

#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__COMMANDS__LINKRBRAIN__PDF_HPP
