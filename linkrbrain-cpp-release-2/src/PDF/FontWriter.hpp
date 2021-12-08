#ifndef LINKRBRAIN2019__SRC__PDF__FONTWRITER_HPP
#define LINKRBRAIN2019__SRC__PDF__FONTWRITER_HPP


#include <map>
#include <set>
#include <filesystem>

#include <hpdf.h>

#include "Logging/Loggable.hpp"
#include "./Style.hpp"


namespace PDF {

    class FontWriter : public Logging::Loggable {
    public:

        FontWriter(HPDF_Doc& hpdf_doc, const std::string& encoding="UTF-8", const bool use_utf_encodings=true) :
            _hpdf_doc(hpdf_doc),
            _encoding(encoding),
            _use_utf_encodings(use_utf_encodings)
        {
            if (_use_utf_encodings) {
                HPDF_UseUTFEncodings(_hpdf_doc);
            }
            HPDF_SetCurrentEncoder(_hpdf_doc, _encoding.c_str());
        }

        const std::string get_logger_name() {
            return "PDF::FontWriter";
        }

    private:

        HPDF_Font get_hpdf_font(const std::string& path_or_name) {
            // if already loaded, don't do anything
            auto it = _hpdf_fonts.find(path_or_name);
            if (it != _hpdf_fonts.end()) {
                get_logger().debug("Font has already been loaded from TTF: ", path_or_name);
                return it->second;
            }
            // if not Base14, extract from file has been requested
            std::string name = path_or_name;
            static const std::set<std::string> base14_fonts_names = {"Courier", "Courier-Bold", "Courier-Oblique", "Courier-BoldOblique", "Helvetica", "Helvetica-Bold", "Helvetica-Oblique", "Helvetica-BoldOblique", "Times-Roman", "Times-Bold", "Times-Italic", "Times-BoldItalic", "Symbol", "ZapfDingbats"};
            if (base14_fonts_names.find(path_or_name) == base14_fonts_names.end()) {
                std::filesystem::path path = path_or_name;
                name = HPDF_LoadTTFontFromFile(_hpdf_doc, path.c_str(), HPDF_TRUE);
            }
            // get font and register in cache
            HPDF_Font hpdf_font = HPDF_GetFont(_hpdf_doc, name.c_str(), _encoding);
            _hpdf_fonts[path_or_name] = hpdf_font;
            _hpdf_fonts[name] = hpdf_font;
            get_logger().debug("Loaded font: ", path_or_name, "; actual name is: ", name);
        }

        std::string _encoding;
        bool _use_utf_encodings;
        HPDF_Doc _hpdf_doc;
        std::map<std::string, HPDF_Font> _hpdf_fonts;

    };

} // PDF


#endif // LINKRBRAIN2019__SRC__PDF__FONTWRITER_HPP
