#ifndef LINKRBRAIN2019__SRC__PDF__FONTMANAGER_HPP
#define LINKRBRAIN2019__SRC__PDF__FONTMANAGER_HPP


#include <hpdf.h>

#include <map>

#include "Logging/Loggable.hpp"

#include "./Font.hpp"
#include "./Style.hpp"


namespace PDF {

    class FontManager : public Logging::Loggable {
    public:

        FontManager(HPDF_Doc& hpdf_doc, const std::string& encoding="UTF-8") :
            _hpdf_doc(hpdf_doc),
            _encoding(encoding) {}

        void load_font(const std::filesystem::path& path) {
            const std::string font_name = HPDF_LoadTTFontFromFile(_hpdf_doc, path.c_str(), HPDF_TRUE);
            const std::string file_name = path.stem();
            std::shared_ptr<Font> font = std::make_shared<Font>(_hpdf_doc, font_name, _encoding);
            _fonts[font_name] = font;
            _fonts[file_name] = font;
            _fonts[path] = font;
            get_logger().debug("Loaded font from file: ", path, "; names are: ", file_name, ", ", font_name);
        }

        Font& get_font(const Style& style) {
            return get_font(style.font_name);
        }

        Font& get_font(const std::string& name) {
            get_logger().detail("Request font: ", name);
            // is it already cached?
            auto it = _fonts.find(name);
            if (it != _fonts.end()) {
                return * it->second;
            }
            // if not, maybe it is a default font
            std::shared_ptr<Font> font = std::make_shared<Font>(_hpdf_doc, name, _encoding);
            _fonts[font->get_name()] = font;
            _fonts[name] = font;
            return * font;
        }

        const float get_cap_height(const Style& style) {
            get_logger().detail("Request cap height for font: ", style.font_name.get_value());
            Font& font = get_font(style.font_name);
            return font.get_cap_height(style);
        }

        const float get_line_height(const Style& style) {
            return style.line_height * get_cap_height(style);
        }

        const std::string get_logger_name() {
            return "PDF::FontManager";
        }

    private:

        HPDF_Doc& _hpdf_doc;
        const std::string _encoding;
        std::map<std::string, std::shared_ptr<Font>> _fonts;

    };

} // PDF


#endif // LINKRBRAIN2019__SRC__PDF__FONTMANAGER_HPP
