#include "./Link.hpp"


#ifndef LINKRBRAIN2019__SRC__PDF__PAGE_HPP
#define LINKRBRAIN2019__SRC__PDF__PAGE_HPP


#include "Logging/Loggable.hpp"
#include "Exceptions/GenericExceptions.hpp"

#include "./Style.hpp"
#include "./FontManager.hpp"

#include <librsvg/rsvg.h>
#include <map>


namespace PDF {

    class Page : public Logging::Loggable {
    public:

        Page(HPDF_Doc& hpdf_doc, FontManager& font_manager, const Style& style, const size_t index) :
            _hpdf_doc(hpdf_doc),
            _font_manager(font_manager),
            _index(index)
        {
            _hpdf_page = HPDF_AddPage(_hpdf_doc);
            HPDF_Page_SetSize(_hpdf_page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
            _height = HPDF_Page_GetHeight(_hpdf_page);
            _width = HPDF_Page_GetWidth(_hpdf_page);
            _scale = _width / 21.f;
            _x = style.margin_left * _scale;
            _y = _height - style.margin_top * _scale;
            if (style.background_color.is_set()) {
                fill(0, 0, 21, 29.7, style.background_color);
            }
        }

        const float get_height() {
            return _height / _scale;
        }
        const float get_width() {
            return _width / _scale;
        }

        // spacing

        const float get_line_height(const Style& style) {
            return _font_manager.get_line_height(style) / _scale;
        }

        const bool is_finished(const Style& style) {
            const float dy = _font_manager.get_line_height(style);
            return _y - dy < style.margin_bottom * _scale;
        }

        void new_line(const Style& style, const float n = 1.f) {
            const float dy = _font_manager.get_line_height(style);
            _x = style.margin_left * _scale;
            _y -= n * dy;
        }

        void new_paragraph(const Style& style) {
            new_line(style, 1.f + style.paragraph_spacing);
        }

        const float get_x() {
            return _x / _scale;
        }
        void set_x(const float& x) {
            _x = _scale * x;
        }
        const float get_y() {
            return _y / _scale;
        }
        void set_y(const float& y) {
            _y = _scale * y;
        }

        // drawing

        void fill(const float x, const float y, const float width, const float height, const Color& color, const bool rescale=true) {
            HPDF_Page_SetLineWidth(_hpdf_page, 0.);
            HPDF_Page_SetRGBFill(_hpdf_page, color.red, color.green, color.blue);
            if (rescale) {
                HPDF_Page_Rectangle(_hpdf_page, x * _scale, y * _scale, width * _scale, height * _scale);
            } else {
                HPDF_Page_Rectangle(_hpdf_page, x, y, width, height);
            }
            HPDF_Page_Fill(_hpdf_page);
        }

        // writing

        void set_font_style(const Style& style = no_style) {
            // validate style
            if (!style.font_name.is_set()) {
                throw Exceptions::BadDataException("Missing font name in style attribute");
            }
            if (!style.font_size.is_set()) {
                throw Exceptions::BadDataException("Missing font size in style attribute");
            }
            // fetch parameters
            Font& font = _font_manager.get_font(style);
            const Color& font_color = style.font_color;
            // apply style to page
            HPDF_Page_SetFontAndSize(_hpdf_page, font.get_hpdf_font(), style.font_size);
            HPDF_Page_SetRGBFill(_hpdf_page, font_color.red, font_color.green, font_color.blue);
            HPDF_Page_SetTextRenderingMode(_hpdf_page, HPDF_FILL);
        }

        size_t get_fitting_characters_count(const std::string& text, const size_t offset, const Style& style, const float available_width, float& written_width) {
            set_font_style(style);
            size_t length = HPDF_Page_MeasureText(
                _hpdf_page,
                text.data() + offset,
                available_width,
                HPDF_TRUE,
                &written_width);
            if (length == 0) {
                length = HPDF_Page_MeasureText(
                    _hpdf_page,
                    text.data() + offset,
                    available_width,
                    HPDF_FALSE,
                    &written_width);
            }
            return length;
        }
        void cut_text(std::vector<std::string>& lines, const std::string& text, float available_width, const Style& style, const bool rescale = false) {
            lines.clear();
            float written_width;
            size_t offset = 0;
            if (rescale) {
                available_width *= _scale;
            }
            while (offset < text.size()) {
                const size_t size = get_fitting_characters_count(text, offset, style, available_width, written_width);
                if (size == 0) {
                    lines.push_back(text.substr(offset));
                    return;
                }
                lines.push_back(text.substr(offset, size));
                offset += size;
            }
        }

        size_t write_until_end_of_line(const std::string& text, const Style& style = no_style, const Link& link = NoLink) {
            // style has to be set beforehands, for proper mesurements
            const float dy = _font_manager.get_line_height(style);
            // how many bytes can be written?
            const float space_left = _width - _x - style.margin_right * _scale;
            float written_width = 0.f;
            size_t length = get_fitting_characters_count(text, 0, style, space_left, written_width);
            // background
            if (style.background_color.is_set()) {
                fill(_x, _y - 1.2*dy, written_width, dy, style.background_color, false);
            }
            // actually write (no need to set font style, because get_fitting_characters_count already does it)
            write_raw(_x, _y - dy, text.substr(0, length), style, false);
            // put a link when requested
            link.make(_hpdf_page, {
                .left = _x,
                .top = _y - 0.2 * dy,
                .right = _x + written_width,
                .bottom = _y - 1.2 * dy,
            });
            // increment x position
            _x += written_width;
            // return number of bytes written
            return length;
        }
        size_t write_until_end_of_page(const std::string& text, const Style& style = no_style, const Link& link = NoLink) {
            std::string line;
            std::stringstream buffer(text);
            bool is_first = true;
            size_t total_written_bytes = 0;
            while (std::getline(buffer, line, '\n')) {
                get_logger().detail("Add text line: `", line, "`");
                // add a new line between extracted lines
                if (is_first) {
                    is_first = false;
                } else {
                    ++total_written_bytes;
                    new_line(style);
                }
                // initialize parameters
                size_t written_bytes = 0;
                while (written_bytes < line.size()) {
                    if (is_finished(style)) {
                        return total_written_bytes;
                    }
                    const std::string subline = line.substr(written_bytes);
                    const size_t line_written_bytes = write_until_end_of_line(subline, style, link);
                    if (line_written_bytes == 0) {
                        return total_written_bytes;
                    }
                    written_bytes += line_written_bytes;
                    total_written_bytes += line_written_bytes;
                    if (written_bytes != line.size()) {
                        new_line(style);
                    }
                }
            }
            return total_written_bytes;
        }

        void write_raw(const float x, const float y, const std::string& text, const Style& style, const bool rescale = true) {
            set_font_style(style);
            if (rescale) {
                write_raw(x * _scale, y * _scale, text);
            } else {
                write_raw(x, y, text);
            }
        }
        void write_raw(const float x, const float y, const std::string& text) {
            HPDF_Page_BeginText(_hpdf_page);
            HPDF_Page_TextOut(_hpdf_page, x, y, text.c_str());
            HPDF_Page_EndText(_hpdf_page);
            get_logger().detail("Page.write_raw(", x, ", ", y, ", ", "`", text, "`)");
        }

        // images

        const float append_image(const Style& style, const HPDF_Image hpdf_image, float width, float height, const Link& link) {
            // recompute width
            if (width == 0.f) {
                width = _width - _x - style.margin_right;
            } else {
                width *= _scale;
            }
            // recompute height
            if (height == 0.f) {
                height = width / HPDF_Image_GetWidth(hpdf_image) * HPDF_Image_GetHeight(hpdf_image);
            } else {
                width *= _scale;
            }
            // draw
            HPDF_Page_DrawImage(_hpdf_page, hpdf_image, _x, _y - height, width, height);
            // put a link when requested
            link.make(_hpdf_page, {
                .left = _x,
                .top = _y,
                .right = _x + width,
                .bottom = _y - height,
            });
            // use drawn height appropriately
            _y -= height;
            return height / _scale;
        }

        //

        HPDF_Page& get_hpdf_page() {
            return _hpdf_page;
        }
        const HPDF_Page& get_hpdf_page() const {
            return _hpdf_page;
        }

        // logging

        virtual const std::string get_logger_name() {
            return "PDF::Page";
        }

    private:

        // references
        HPDF_Doc& _hpdf_doc;
        HPDF_Page _hpdf_page;
        FontManager& _font_manager;
        // page index
        size_t _index;
        // numeric properties
        float _width;
        float _height;
        float _scale;
        float _x;
        float _y;
    };

} // PDF


#endif // LINKRBRAIN2019__SRC__PDF__PAGE_HPP
