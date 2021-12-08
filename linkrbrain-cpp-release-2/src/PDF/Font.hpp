#ifndef LINKRBRAIN2019__SRC__PDF__FONT_HPP
#define LINKRBRAIN2019__SRC__PDF__FONT_HPP


#include <map>

#include <hpdf.h>

#include "Logging/Loggable.hpp"
#include "./Style.hpp"


namespace PDF {

    class Font : public Logging::Loggable {
    public:

        Font(HPDF_Doc hpdf_doc, const std::string &name, const std::string& encoding="UTF-8") :
            _hpdf_doc(hpdf_doc),
            _name(name),
            _encoding(encoding)
        {
            _hpdf_font = HPDF_GetFont(_hpdf_doc, name.c_str(), _encoding.c_str());
            _name = HPDF_Font_GetFontName(_hpdf_font);
        }

        const std::string& get_name() {
            return _name;
        }

        HPDF_Font get_hpdf_font() {
            return _hpdf_font;
        }

        const float get_cap_height(const Style& style) {
            // is it cached?
            const auto it = _cap_heights.find(style.font_size);
            if (it != _cap_heights.end()) {
                return it->second;
            }
            // if not, compute it
            const float cap_height = HPDF_Font_GetCapHeight(_hpdf_font) * style.font_size / 1000.f;
            _cap_heights[style.font_size] = cap_height;
            return cap_height;
        }

        virtual const std::string get_logger_name() {
            return "PDF::Font[" + _name + "]";
        }

    private:

        HPDF_Doc _hpdf_doc;
        std::string _name;
        std::string _encoding;
        HPDF_Font _hpdf_font;
        std::map<float, float> _cap_heights;

    };

} // PDF


#endif // LINKRBRAIN2019__SRC__PDF__FONT_HPP
