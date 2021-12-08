#include "PDF/Document.hpp"

#include <iostream>


int main(int argc, char const *argv[]) {

    PDF::Document pdf;

    pdf.load_font("var/fonts/ubuntu/Ubuntu-R.ttf");
    pdf.set_style({.font_name = "Ubuntu-R"});

    const float y = pdf.get_y();
    pdf.set_y(y + 5);
    pdf.append_image("var/images/logo_footer_resized.png", 25);
    pdf.set_y(y);

    pdf.start_link("https://www.linkrbrain.org");
    pdf.append_text("LinkRbrain", {.font_color = {0,0,1}});
    pdf.stop_link();
    pdf.append_text(" is the best neuroscience platform!");

    pdf.new_paragraph();
    pdf.start_link("https://www.linkrbrain.org");
    pdf.append_image("var/images/logo_header_resized.png", 10);
    pdf.stop_link();

    pdf.new_page();
    pdf.append_text("This is the second page.");
    pdf.new_page();
    pdf.append_text("This is the third page.");
    pdf.new_page();
    pdf.append_image("var/images/logo_watermark.png", 25);

    pdf.set_current_page_index(0);

    pdf.new_paragraph();
    pdf.start_link(1);
    pdf.append_text("Go to page two");
    pdf.stop_link();

    pdf.new_paragraph();
    pdf.start_link(2);
    pdf.append_text("Go to page three");
    pdf.stop_link();

    pdf.save("/tmp/test.pdf");

    // // format link
    // const std::string link_text = "LinkRbrain";
    // const std::string link_target = "https://www.linkrbrain.org";
    // PDF::Link link(link_text, link_target);
    // const std::string formatted = link.get_formatted();
    // std::cout << formatted << '\n';
    //
    // // parse link
    // PDF::Link link2(formatted);
    // std::cout << link2.get_text() << '\n';
    // std::cout << link2.get_target() << '\n';
    //
    // // test if strings are links
    // std::cout << formatted << " is " << (PDF::Link::is_link(formatted) ? "" : "not ") << "a link" << '\n';
    // std::cout << "[truc]" << " is " << (PDF::Link::is_link("[truc]") ? "" : "not ") << "a link" << '\n';

    // the end!
    return 0;
}
