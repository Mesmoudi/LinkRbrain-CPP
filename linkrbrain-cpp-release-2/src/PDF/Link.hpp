#ifndef LINKRBRAIN2019__SRC__PDF__LINK_HPP
#define LINKRBRAIN2019__SRC__PDF__LINK_HPP


#include <string>


namespace PDF {

    class Page;

    class Link {
    public:

        enum Type {None, Internal, External};

        Link() :
            _type(None) {}

        Link(const Page& page) :
            _type(Internal),
            _page(&page) {}

        Link(const std::string& uri) :
            _type(External),
            _uri(uri) {}

        Link(const char* uri) :
            _type(External),
            _uri(uri) {}

        //

        void unset() {
            _type = None;
        }

        void set(const Page& page) {
            _type = Internal;
            _page = &page;
        }

        void set(const std::string& uri) {
            _type = External;
            _uri = uri;
        }

        //

        operator const bool () const {
            return (_type != None);
        }

        //

        void make(HPDF_Page& hpdf_page, const HPDF_Rect& hpdf_rect) const;

    private:

        Type _type;
        const Page* _page;
        std::string _uri;

    };

    static const Link NoLink;

}


#include "./Page.hpp"


void PDF::Link::make(HPDF_Page& hpdf_page, const HPDF_Rect& hpdf_rect) const {
    HPDF_Annotation hpdf_annotation;
    switch (_type) {
        case External:
            hpdf_annotation = HPDF_Page_CreateURILinkAnnot(hpdf_page, hpdf_rect, _uri.c_str());
            break;
        case Internal:
            hpdf_annotation = HPDF_Page_CreateLinkAnnot(hpdf_page, hpdf_rect, HPDF_Page_CreateDestination(_page->get_hpdf_page()));
            break;
        case None:
            return;
    }
    HPDF_LinkAnnot_SetHighlightMode(hpdf_annotation, HPDF_ANNOT_NO_HIGHTLIGHT);
    HPDF_LinkAnnot_SetBorderStyle(hpdf_annotation, 0.0, 0, 0);
}


#endif // LINKRBRAIN2019__SRC__PDF__LINK_HPP
