#ifndef LINKRBRAIN2019__SRC__PDF__DOCUMENT_HPP
#define LINKRBRAIN2019__SRC__PDF__DOCUMENT_HPP


#include "Exceptions/Exception.hpp"
#include "Exceptions/GenericExceptions.hpp"

#include <hpdf.h>

#include <filesystem>
#include <fstream>
#include <vector>
#include <numeric>

#include "Logging/Loggable.hpp"
#include "Types/Table.hpp"

#include "./Style.hpp"
#include "./Page.hpp"
#include "./FontManager.hpp"


namespace PDF {

    class Document : Logging::Loggable {
    public:

        // construction / destruction

        Document(const Style& style=no_style, const std::string& encoding="UTF-8") :
            _style(default_style | style),
            _encoding(encoding),
            _hpdf_doc(HPDF_New(hpdf_error_callback, this)),
            _font_manager(_hpdf_doc, encoding),
            _current_page_index(-1),
            _current_page(NULL)
        {
            // check document
            if (!_hpdf_doc) {
                throw Exceptions::Exception("Could not create Document instance");
            }
            if (!HPDF_HasDoc(_hpdf_doc)) {
                throw Exceptions::Exception("Invalid PDF document");
            }
            // set encoding
            HPDF_UseUTFEncodings(_hpdf_doc);
            HPDF_SetCurrentEncoder(_hpdf_doc, "UTF-8");
            HPDF_SetCompressionMode(_hpdf_doc, HPDF_COMP_ALL);
            get_logger().notice("Set document encoding");
        }

        ~Document() {
            get_logger().debug("Closing document");
            HPDF_Free(_hpdf_doc);
            get_logger().notice("Closed document");
        }

        // style

        const Style& get_style() {
            return _style;
        }

        void set_style(const Style& style, std::initializer_list<std::string>& keys) {
            _style.get_child(keys) |= style;
        }
        void set_style(const Style& style, const std::string& key) {
            _style.get_child({key}) |= style;
        }
        void set_style(const Style& style) {
            _style |= style;
        }

        // fonts

        void load_font(const std::filesystem::path& path) {
            _font_manager.load_font(path);
        }

        // linking

        void start_link(const std::string& uri) {
            _current_link.set(uri);
        }
        void start_link(const Page& page) {
            _current_link.set(page);
        }
        void start_link(const size_t page_index) {
            if (page_index >= _pages.size()) {
                throw Exceptions::BadDataException("Page index greater than possible: " + std::to_string(page_index) + ", should be smaller than " + std::to_string(_pages.size()));
            }
            return start_link(_pages[page_index]);
        }
        void stop_link() {
            _current_link.unset();
        }

        // writing things

        void append_text(const std::string& text, const Style& style = no_style) {
            get_logger().detail("Add text: `", text, "`");
            const Style computed_style = _style.get_computed() | style;
            // write until end of page is reached, then do it again in a new page
            size_t total_written_bytes = 0;
            while (total_written_bytes < text.size()) {
                size_t written_bytes = 0;
                if (total_written_bytes) {
                    new_page(computed_style);
                    written_bytes = get_current_page().write_until_end_of_page(text.substr(total_written_bytes), computed_style, _current_link);
                } else {
                    written_bytes = get_current_page().write_until_end_of_page(text, computed_style, _current_link);
                }
                if (written_bytes == 0) {
                    new_page(computed_style);
                }
                total_written_bytes += written_bytes;
            }
        }

        void append_paragraph(const std::string& text, const Style& style = no_style) {
            append_text(text, style);
            new_paragraph(style);
        }
        void append_line(const std::string& text, const Style& style = no_style) {
            append_text(text, style);
            new_line(style);
        }

        // about tables

        void append_table_row(const std::vector<std::string>& row, const std::vector<float>& widths, const Style& style) {
            // compute constants
            const Style computed_style = _style | style;
            const float total_width = std::accumulate(widths.begin(), widths.end(), 0.f);
            const float line_height = get_current_page().get_line_height(computed_style);
            const float x0 = get_current_page().get_x();
            // cut lines within rows appropriately
            std::vector<std::vector<std::string>> row_lines;
            const size_t max_lines_count = cut_table_row(row_lines, row, widths, style);
            // background before
            if (style.margin_top.is_set()) {
                if (computed_style.background_color.is_set()) {
                    get_current_page().fill(x0, get_current_page().get_y() - style.margin_top, total_width, style.margin_top + 0.01, computed_style.background_color);
                }
                increment_y(style.margin_top);
            }
            // render line by line...
            for (size_t l = 0; l < max_lines_count; l++) {
                // render background...
                if (computed_style.background_color.is_set()) {
                    get_current_page().fill(x0, get_current_page().get_y() - line_height, total_width, line_height + 0.01, computed_style.background_color);
                }
                // ...and text, cell by cell
                float x = x0;
                for (size_t i = 0, n = widths.size(); i < n; i++) {
                    if (i < row_lines.size() && l < row_lines[i].size()) {
                        const std::string& line = row_lines[i][l];
                        const float dx = style.margin_left.is_set() ? style.margin_left.get_value() : 0.f;
                        const float dy = 0.3 * line_height - line_height;
                        get_current_page().write_raw(x + dx, get_current_page().get_y() + dy, line, computed_style);
                    }
                    x += widths[i];
                }
                // move further down
                increment_y(line_height);
            }
            // background after
            if (style.margin_bottom.is_set()) {
                if (computed_style.background_color.is_set()) {
                    get_current_page().fill(x0, get_current_page().get_y() - style.margin_bottom, total_width, style.margin_bottom + 0.01, computed_style.background_color);
                }
                increment_y(style.margin_bottom);
            }
        }
        const size_t cut_table_row(std::vector<std::vector<std::string>>& row_lines, const std::vector<std::string>& row, const std::vector<float>& widths, const Style& style) {
            row_lines.clear();
            row_lines.resize(std::max(widths.size(), row.size()));
            size_t max_lines_count = 0;
            for (size_t i = 0, n = std::min(widths.size(), row.size()); i < n; i++) {
                std::vector<std::string>& cell_lines = row_lines[i];
                const float width = widths[i] - style.margin_left - style.margin_right;
                get_current_page().cut_text(cell_lines, row[i], width, _style | style, true);
                if (cell_lines.size() > max_lines_count) {
                    max_lines_count = cell_lines.size();
                }
            }
            return max_lines_count;
        }

        void append_table(const Types::Table& table, std::vector<float> widths = {}, const Style& row_style = no_style) {
            append_table(table, widths, row_style, row_style, row_style);
        }
        void append_table(const Types::Table& table, std::vector<float> widths, const Style& header_style, const Style& row_style) {
            append_table(table, widths, header_style, row_style, row_style);
        }
        void append_table(const Types::Table& table, std::vector<float> widths, const Style& header_style, const Style& even_style, const Style& odd_style) {
            // determine width for each column
            const size_t columns_count = table.get_column_count();
            if (widths.size() < columns_count) {
                const float available_width = get_current_page().get_width() - _style.margin_left - _style.margin_right;
                float width_sum = 0.f;
                for (const float column_width : widths) {
                    width_sum += column_width;
                }
                const float column_width = (available_width - width_sum) / (float) (columns_count - widths.size());
                for (size_t i = widths.size(); i < columns_count; i++) {
                    widths.push_back(column_width);
                }
            }
            // render table
            if (table.get_header().size() > 0) {
                // start a new page if necessary
                if (get_current_page().is_finished(_style | header_style)) {
                    new_page();
                }
                // append header row
                append_table_row(table.get_header(), widths, header_style);
            }
            for (size_t i = 0, n = table.get_rows_count(); i < n; i++) {
                const Style& row_style = (i % 2) ? odd_style : even_style;
                // start a new page if necessary
                if (get_current_page().is_finished(_style | row_style)) {
                    new_page();
                    append_table_row(table.get_header(), widths, header_style);
                }
                // append body row
                append_table_row(table.get_row(i), widths, row_style);
            }
            get_current_page().new_line(_style);
        }

        // separator

        void append_separator(const Color& color = {.5,.5,.5}, const float height = 0.01) {
            if (get_current_page().get_x() != _style.margin_left) {
                new_line();
            }
            new_line();
            const float x = _style.margin_left;
            const float y = get_current_page().get_y();
            const float width = get_current_page().get_width() - _style.margin_left - _style.margin_right;
            get_current_page().fill(x, y, width, height, color);
            get_current_page().set_y(
                get_current_page().get_y() + height
            );
            new_line();
        }

        // triggers

        virtual void on_after_new_page() {}

        // pagination

        void new_line(const Style& style = no_style) {
            get_current_page().new_line(_style.get_computed() | style);
        }
        void new_paragraph(const Style& style = no_style) {
            get_current_page().new_paragraph(_style.get_computed() | style);
        }
        Page& new_page(const Style& style = no_style) {
            // add a new page if this is the last one
            if (_current_page_index == _pages.size() - 1) {
                _pages.push_back({
                    _hpdf_doc,
                    _font_manager,
                    _style.get_computed() | style,
                    _pages.size()
                });
            }
            _current_page_index += 1;
            _current_page = & _pages[_current_page_index];
            //
            this->on_after_new_page();
            //
            return * _current_page;
        }

        const float get_x(const Style& style = no_style) {
            const Style computed_style = _style.get_computed() | style;
            return get_current_page().get_x() - computed_style.margin_left;
        }
        void set_x(const float x, const Style& style = no_style) {
            const Style computed_style = _style.get_computed() | style;
            get_current_page().set_x(computed_style.margin_left + x);
        }
        void increment_x(const float dx) {
            auto& current_page = get_current_page();
            current_page.set_x(current_page.get_x() + dx);
        }

        const float get_y(const Style& style = no_style) {
            const Style computed_style = _style.get_computed() | style;
            auto& current_page = get_current_page();
            return current_page.get_height() - current_page.get_y() - computed_style.margin_top;
        }
        void set_y(const float y, const Style& style = no_style) {
            const Style computed_style = _style.get_computed() | style;
            auto& current_page = get_current_page();
            current_page.set_y(current_page.get_height() - computed_style.margin_top - y);
        }
        void increment_y(const float dy) {
            auto& current_page = get_current_page();
            current_page.set_y(current_page.get_y() - dy);
        }

        const std::pair<float, float> get_xy() {
            return {get_x(), get_y()};
        }
        void set_xy(const float x,  const float y) {
            set_x(x);
            set_y(y);
        }

        // pictures

        HPDF_Image load_image_svg(std::ifstream& file, const float width, const std::filesystem::path& path) {
            // load file contents
            std::string text;
            std::getline(file, text, '\0');
            // parse SVG
            GError* error = NULL;
            RsvgHandle* rsvg_handle = rsvg_handle_new_from_data((const guint8*)text.data(), (gsize)text.size(), &error);
            if (error != NULL) {
                const std::string librsvg_message = error->message;
                g_clear_object(&rsvg_handle);
                throw Exceptions::BadDataException("Cannot parse SVG file `" + path.native() + "` with librsvg: " + librsvg_message);
            }
            RsvgDimensionData rsvg_dimensions;
            rsvg_handle_get_dimensions(rsvg_handle, &rsvg_dimensions);
            // paint SVG onto Cairo image
            cairo_surface_t* cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, rsvg_dimensions.width, rsvg_dimensions.height);
            cairo_t* cairo_context = cairo_create(cairo_surface);
            rsvg_handle_render_cairo(rsvg_handle, cairo_context);
            cairo_status_t cairo_state = cairo_status(cairo_context);
            if (cairo_state) {
                const std::string cairo_message = cairo_status_to_string(cairo_state);
                cairo_destroy(cairo_context);
                cairo_surface_destroy(cairo_surface);
                throw Exceptions::BadDataException("Cannot paint SVG file `" + path.native() + "` onto Cairo surface: " + cairo_message);
            }
            // convert Cairo image to in-memory PNG
            std::string png_buffer;
            cairo_surface_write_to_png_stream(cairo_surface, [] (void* png_buffer_, const unsigned char* data, unsigned int size) {
                std::string& png_buffer = * (std::string*) png_buffer_;
                png_buffer.append((const char*) data, size);
                return CAIRO_STATUS_SUCCESS;
            }, &png_buffer);
            // convert Cairo image to libharu image
            HPDF_Image image = HPDF_LoadPngImageFromMem(_hpdf_doc, (HPDF_BYTE*) png_buffer.data(), png_buffer.size());
            // cleanup & return
            g_clear_object(&rsvg_handle);
            cairo_destroy(cairo_context);
            cairo_surface_destroy(cairo_surface);
            return image;
        }
        HPDF_Image load_image(const std::filesystem::path& path, const float width) {
            std::string prefix(8UL, '\0');
            std::ifstream file(path, std::ios::binary);
            if (!file.good()) {
                throw Exceptions::BadDataException("Could not open file: " + path.native());
            }
            file.read(prefix.data(), 8);
            HPDF_Image hpdf_image;
            // load image
            if (prefix == "\x89PNG\x0D\x0A\x1A\x0A") {
                hpdf_image = HPDF_LoadPngImageFromFile(_hpdf_doc, path.c_str());
            } else if (prefix.substr(0, 3) == "\xFF\xD8\xFF") {
                hpdf_image = HPDF_LoadJpegImageFromFile(_hpdf_doc, path.c_str());
            } else if (prefix == "<?xml ve" || prefix.substr(0, 4) == "<svg" || prefix.substr(0, 4) == "<SVG") {
                file.seekg(0);
                hpdf_image = load_image_svg(file, width, path);
            } else {
                throw Exceptions::BadDataException("Unrecognized image format: " + path.native());
            }
            // alpha channel when requested (see https://github.com/dafixer/libharu/commit/e68711c1419b611c8cac736fdf306cd280544523)
            if (hpdf_image && (_hpdf_doc->compression_mode & HPDF_COMP_IMAGE)) {
                HPDF_Dict hpdf_mask = (HPDF_Dict) HPDF_Dict_GetItem(hpdf_image, "SMask", HPDF_OCLASS_DICT);
                if (hpdf_mask) {
                    hpdf_mask->filter = HPDF_STREAM_FILTER_FLATE_DECODE;
                }
            }
            // the end!
            return hpdf_image;
        }
        HPDF_Image load_image_with_caching(const std::filesystem::path& path, const float width) {
            auto it = _images_cache.find(path);
            if (it != _images_cache.end()) {
                return it->second;
            }
            HPDF_Image hpdf_image = load_image(path, width);
            _images_cache.insert({path, hpdf_image});
            return hpdf_image;
        }

        const float append_image(const HPDF_Image& hpdf_image, float width = 0.f, float height = 0.f) {
            return get_current_page().append_image(_style.get_computed(), hpdf_image, width, height, _current_link);
        }
        const float append_image(const std::filesystem::path& path, float width = 0.f, float height = 0.f) {
            HPDF_Image hpdf_image = load_image_with_caching(path, width);
            return append_image(hpdf_image, width, height);
        }

        // save file

        void save(const std::filesystem::path& path, const bool compress=true) {
            HPDF_SaveToFile(_hpdf_doc, path.c_str());
            get_logger().message("Saved document to ", path);
        }

        void save(std::ostream& output, const bool compress=true) {
            // compression
            if (compress) {
                HPDF_SetCompressionMode(_hpdf_doc, HPDF_COMP_ALL);
            } else {
                HPDF_SetCompressionMode(_hpdf_doc, HPDF_COMP_NONE);
            }
            get_logger().debug("Preparing to save, with", compress?"":"out", " compression");
            HPDF_SaveToStream(_hpdf_doc);
            get_logger().debug("Preparing to save, size is ", HPDF_GetStreamSize(_hpdf_doc));
            HPDF_ResetStream(_hpdf_doc);
            get_logger().debug("Resetted stream");
            for (;;) {
                HPDF_BYTE buffer[4096];
                HPDF_UINT32 size = 4096;
                HPDF_STATUS status = HPDF_ReadFromStream(_hpdf_doc, buffer, &size);
                if (size == 0) {
                    break;
                }
                output.write((const char*) buffer, size);
            }
        }

        // browse pages

        const size_t get_page_count() const {
            return _pages.size();
        }

        void set_current_page_index(const size_t page_index) {
            if (page_index >= _pages.size()) {
                throw Exceptions::BadDataException("Cannot set current page index to " + std::to_string(page_index) + ", maximum page index is " + std::to_string(_pages.size() - 1));
            }
            _current_page_index = page_index;
            _current_page = & _pages[_current_page_index];
        }
        const size_t get_current_page_index() {
            return _current_page_index;
        }

        // logging

        const std::string get_logger_name() {
            return "PDF::Document";
        }

    protected:

        Page& get_current_page() {
            if (_current_page == NULL) {
                return new_page();
            }
            return *_current_page;
        }

        HPDF_Doc _hpdf_doc;

    private:

        static const std::string hpdf_error_name(HPDF_STATUS hpdf_status) {
            switch (hpdf_status) {
                case 0x1001: return "HPDF_ARRAY_COUNT_ERR";
                case 0x1002: return "HPDF_ARRAY_ITEM_NOT_FOUND";
                case 0x1003: return "HPDF_ARRAY_ITEM_UNEXPECTED_TYPE";
                case 0x1004: return "HPDF_BINARY_LENGTH_ERR";
                case 0x1005: return "HPDF_CANNOT_GET_PALLET";
                case 0x1007: return "HPDF_DICT_COUNT_ERR";
                case 0x1008: return "HPDF_DICT_ITEM_NOT_FOUND";
                case 0x1009: return "HPDF_DICT_ITEM_UNEXPECTED_TYPE";
                case 0x100A: return "HPDF_DICT_STREAM_LENGTH_NOT_FOUND";
                case 0x100B: return "HPDF_DOC_ENCRYPTDICT_NOT_FOUND";
                case 0x100C: return "HPDF_DOC_INVALID_OBJECT";
                /*                                                0x100D */
                case 0x100E: return "HPDF_DUPLICATE_REGISTRATION";
                case 0x100F: return "HPDF_EXCEED_JWW_CODE_NUM_LIMIT";
                /*                                                0x1010 */
                case 0x1011: return "HPDF_ENCRYPT_INVALID_PASSWORD";
                /*                                                0x1012 */
                case 0x1013: return "HPDF_ERR_UNKNOWN_CLASS";
                case 0x1014: return "HPDF_EXCEED_GSTATE_LIMIT";
                case 0x1015: return "HPDF_FAILD_TO_ALLOC_MEM";
                case 0x1016: return "HPDF_FILE_IO_ERROR";
                case 0x1017: return "HPDF_FILE_OPEN_ERROR";
                /*                                                0x1018 */
                case 0x1019: return "HPDF_FONT_EXISTS";
                case 0x101A: return "HPDF_FONT_INVALID_WIDTHS_TABLE";
                case 0x101B: return "HPDF_INVALID_AFM_HEADER";
                case 0x101C: return "HPDF_INVALID_ANNOTATION";
                /*                                                0x101D */
                case 0x101E: return "HPDF_INVALID_BIT_PER_COMPONENT";
                case 0x101F: return "HPDF_INVALID_CHAR_MATRICS_DATA";
                case 0x1020: return "HPDF_INVALID_COLOR_SPACE";
                case 0x1021: return "HPDF_INVALID_COMPRESSION_MODE";
                case 0x1022: return "HPDF_INVALID_DATE_TIME";
                case 0x1023: return "HPDF_INVALID_DESTINATION";
                /*                                                0x1024 */
                case 0x1025: return "HPDF_INVALID_DOCUMENT";
                case 0x1026: return "HPDF_INVALID_DOCUMENT_STATE";
                case 0x1027: return "HPDF_INVALID_ENCODER";
                case 0x1028: return "HPDF_INVALID_ENCODER_TYPE";
                /*                                                0x1029 */
                /*                                                0x102A */
                case 0x102B: return "HPDF_INVALID_ENCODING_NAME";
                case 0x102C: return "HPDF_INVALID_ENCRYPT_KEY_LEN";
                case 0x102D: return "HPDF_INVALID_FONTDEF_DATA";
                case 0x102E: return "HPDF_INVALID_FONTDEF_TYPE";
                case 0x102F: return "HPDF_INVALID_FONT_NAME";
                case 0x1030: return "HPDF_INVALID_IMAGE";
                case 0x1031: return "HPDF_INVALID_JPEG_DATA";
                case 0x1032: return "HPDF_INVALID_N_DATA";
                case 0x1033: return "HPDF_INVALID_OBJECT";
                case 0x1034: return "HPDF_INVALID_OBJ_ID";
                case 0x1035: return "HPDF_INVALID_OPERATION";
                case 0x1036: return "HPDF_INVALID_OUTLINE";
                case 0x1037: return "HPDF_INVALID_PAGE";
                case 0x1038: return "HPDF_INVALID_PAGES";
                case 0x1039: return "HPDF_INVALID_PARAMETER";
                /*                                                0x103A */
                case 0x103B: return "HPDF_INVALID_PNG_IMAGE";
                case 0x103C: return "HPDF_INVALID_STREAM";
                case 0x103D: return "HPDF_MISSING_FILE_NAME_ENTRY";
                /*                                                0x103E */
                case 0x103F: return "HPDF_INVALID_TTC_FILE";
                case 0x1040: return "HPDF_INVALID_TTC_INDEX";
                case 0x1041: return "HPDF_INVALID_WX_DATA";
                case 0x1042: return "HPDF_ITEM_NOT_FOUND";
                case 0x1043: return "HPDF_LIBPNG_ERROR";
                case 0x1044: return "HPDF_NAME_INVALID_VALUE";
                case 0x1045: return "HPDF_NAME_OUT_OF_RANGE";
                /*                                                0x1046 */
                /*                                                0x1047 */
                case 0x1048: return "HPDF_PAGE_INVALID_PARAM_COUNT";
                case 0x1049: return "HPDF_PAGES_MISSING_KIDS_ENTRY";
                case 0x104A: return "HPDF_PAGE_CANNOT_FIND_OBJECT";
                case 0x104B: return "HPDF_PAGE_CANNOT_GET_ROOT_PAGES";
                case 0x104C: return "HPDF_PAGE_CANNOT_RESTORE_GSTATE";
                case 0x104D: return "HPDF_PAGE_CANNOT_SET_PARENT";
                case 0x104E: return "HPDF_PAGE_FONT_NOT_FOUND";
                case 0x104F: return "HPDF_PAGE_INVALID_FONT";
                case 0x1050: return "HPDF_PAGE_INVALID_FONT_SIZE";
                case 0x1051: return "HPDF_PAGE_INVALID_GMODE";
                case 0x1052: return "HPDF_PAGE_INVALID_INDEX";
                case 0x1053: return "HPDF_PAGE_INVALID_ROTATE_VALUE";
                case 0x1054: return "HPDF_PAGE_INVALID_SIZE";
                case 0x1055: return "HPDF_PAGE_INVALID_XOBJECT";
                case 0x1056: return "HPDF_PAGE_OUT_OF_RANGE";
                case 0x1057: return "HPDF_REAL_OUT_OF_RANGE";
                case 0x1058: return "HPDF_STREAM_EOF";
                case 0x1059: return "HPDF_STREAM_READLN_CONTINUE";
                /*                                                0x105A */
                case 0x105B: return "HPDF_STRING_OUT_OF_RANGE";
                case 0x105C: return "HPDF_THIS_FUNC_WAS_SKIPPED";
                case 0x105D: return "HPDF_TTF_CANNOT_EMBEDDING_FONT";
                case 0x105E: return "HPDF_TTF_INVALID_CMAP";
                case 0x105F: return "HPDF_TTF_INVALID_FOMAT";
                case 0x1060: return "HPDF_TTF_MISSING_TABLE";
                case 0x1061: return "HPDF_UNSUPPORTED_FONT_TYPE";
                case 0x1062: return "HPDF_UNSUPPORTED_FUNC";
                case 0x1063: return "HPDF_UNSUPPORTED_JPEG_FORMAT";
                case 0x1064: return "HPDF_UNSUPPORTED_TYPE1_FONT";
                case 0x1065: return "HPDF_XREF_COUNT_ERR";
                case 0x1066: return "HPDF_ZLIB_ERROR";
                case 0x1067: return "HPDF_INVALID_PAGE_INDEX";
                case 0x1068: return "HPDF_INVALID_URI";
                case 0x1069: return "HPDF_PAGE_LAYOUT_OUT_OF_RANGE";
                case 0x1070: return "HPDF_PAGE_MODE_OUT_OF_RANGE";
                case 0x1071: return "HPDF_PAGE_NUM_STYLE_OUT_OF_RANGE";
                case 0x1072: return "HPDF_ANNOT_INVALID_ICON";
                case 0x1073: return "HPDF_ANNOT_INVALID_BORDER_STYLE";
                case 0x1074: return "HPDF_PAGE_INVALID_DIRECTION";
                case 0x1075: return "HPDF_INVALID_FONT";
                case 0x1076: return "HPDF_PAGE_INSUFFICIENT_SPACE";
                case 0x1077: return "HPDF_PAGE_INVALID_DISPLAY_TIME";
                case 0x1078: return "HPDF_PAGE_INVALID_TRANSITION_TIME";
                case 0x1079: return "HPDF_INVALID_PAGE_SLIDESHOW_TYPE";
                case 0x1080: return "HPDF_EXT_GSTATE_OUT_OF_RANGE";
                case 0x1081: return "HPDF_INVALID_EXT_GSTATE";
                case 0x1082: return "HPDF_EXT_GSTATE_READ_ONLY";
                case 0x1083: return "HPDF_INVALID_U3D_DATA";
                case 0x1084: return "HPDF_NAME_CANNOT_GET_NAMES";
                case 0x1085: return "HPDF_INVALID_ICC_COMPONENT_NUM";
            }
            return "[" + std::to_string(hpdf_status) + "]";
        }
        static const std::string hpdf_error_message(HPDF_STATUS hpdf_status) {
            switch (hpdf_status) {
                case HPDF_ARRAY_COUNT_ERR: // 0x1001
                    return "0x1001: Internal error. The consistency of the data was lost.";
                case HPDF_ARRAY_ITEM_NOT_FOUND: // 0x1002
                    return "0x1002: Internal error. The consistency of the data was lost.";
                case HPDF_ARRAY_ITEM_UNEXPECTED_TYPE: // 0x1003
                    return "0x1003: Internal error. The consistency of the data was lost.";
                case HPDF_BINARY_LENGTH_ERR: // 0x1004
                    return "0x1004: The length of the data exceeds HPDF_LIMIT_MAX_STRING_LEN.";
                case HPDF_CANNOT_GET_PALLET: // 0x1005
                    return "0x1005: Cannot get a pallet data from PNG image.";
                case HPDF_DICT_COUNT_ERR: // 0x1007
                    return "0x1007: The count of elements of a dictionary exceeds HPDF_LIMIT_MAX_DICT_ELEMENT";
                case HPDF_DICT_ITEM_NOT_FOUND: // 0x1008
                    return "0x1008: Internal error. The consistency of the data was lost.";
                case HPDF_DICT_ITEM_UNEXPECTED_TYPE: // 0x1009
                    return "0x1009: Internal error. The consistency of the data was lost.";
                case HPDF_DICT_STREAM_LENGTH_NOT_FOUND: // 0x100A
                    return "0x100A: Internal error. The consistency of the data was lost.";
                case HPDF_DOC_ENCRYPTDICT_NOT_FOUND: // 0x100B
                    return "0x100B: HPDF_SetPermission() OR HPDF_SetEncryptMode() was called before a password is set.";
                case HPDF_DOC_INVALID_OBJECT: // 0x100C
                    return "0x100C: Internal error. The consistency of the data was lost.";
                case HPDF_DUPLICATE_REGISTRATION: // 0x100E
                    return "0x100E: Tried to register a font that has been registered.";
                case HPDF_EXCEED_JWW_CODE_NUM_LIMIT: // 0x100F
                    return "0x100F: Cannot register a character to the japanese word wrap characters list.";
                case HPDF_ENCRYPT_INVALID_PASSWORD: // 0x1011
                    return "0x1011: Tried to set the owner password to NULL. The owner password and user password is the same.";
                case HPDF_ERR_UNKNOWN_CLASS: // 0x1013
                    return "0x1013: Internal error. The consistency of the data was lost.";
                case HPDF_EXCEED_GSTATE_LIMIT: // 0x1014
                    return "0x1014: The depth of the stack exceeded HPDF_LIMIT_MAX_GSTATE.";
                case HPDF_FAILD_TO_ALLOC_MEM: // 0x1015
                    return "0x1015: Memory allocation failed.";
                case HPDF_FILE_IO_ERROR: // 0x1016
                    return "0x1016: File processing failed. (A detailed code is set.)";
                case HPDF_FILE_OPEN_ERROR: // 0x1017
                    return "0x1017: Cannot open a file. (A detailed code is set.)";
                case HPDF_FONT_EXISTS: // 0x1019
                    return "0x1019: Tried to load a font that has been registered.";
                case HPDF_FONT_INVALID_WIDTHS_TABLE: // 0x101A
                    return "0x101A: The format of a font-file is invalid. Internal error. The consistency of the data was lost.";
                case HPDF_INVALID_AFM_HEADER: // 0x101B
                    return "0x101B: Cannot recognize a header of an afm file.";
                case HPDF_INVALID_ANNOTATION: // 0x101C
                    return "0x101C: The specified annotation handle is invalid.";
                case HPDF_INVALID_BIT_PER_COMPONENT: // 0x101E
                    return "0x101E: Bit-per-component of a image which was set as mask-image is invalid.";
                case HPDF_INVALID_CHAR_MATRICS_DATA: // 0x101F
                    return "0x101F: Cannot recognize char-matrics-data  of an afm file.";
                case HPDF_INVALID_COLOR_SPACE: // 0x1020
                    return "0x1020: 1. The color_space parameter of HPDF_LoadRawImage is invalid. 2. Color-space of a image which was set as mask-image is invalid. 3. The function which is invalid in the present color-space was invoked.";
                case HPDF_INVALID_COMPRESSION_MODE: // 0x1021
                    return "0x1021: Invalid value was set when invoking HPDF_SetCommpressionMode().";
                case HPDF_INVALID_DATE_TIME: // 0x1022
                    return "0x1022: An invalid date-time value was set.";
                case HPDF_INVALID_DESTINATION: // 0x1023
                    return "0x1023: An invalid destination handle was set.";
                case HPDF_INVALID_DOCUMENT: // 0x1025
                    return "0x1025: An invalid document handle is set.";
                case HPDF_INVALID_DOCUMENT_STATE: // 0x1026
                    return "0x1026: The function which is invalid in the present state was invoked.";
                case HPDF_INVALID_ENCODER: // 0x1027
                    return "0x1027: An invalid encoder handle is set.";
                case HPDF_INVALID_ENCODER_TYPE: // 0x1028
                    return "0x1028: A combination between font and encoder is wrong.";
                case HPDF_INVALID_ENCODING_NAME: // 0x102B
                    return "0x102B: An Invalid encoding name is specified.";
                case HPDF_INVALID_ENCRYPT_KEY_LEN: // 0x102C
                    return "0x102C: The lengh of the key of encryption is invalid.";
                case HPDF_INVALID_FONTDEF_DATA: // 0x102D
                    return "0x102D: 1. An invalid font handle was set. 2. Unsupported font format.";
                case HPDF_INVALID_FONTDEF_TYPE: // 0x102E
                    return "0x102E: Internal error. The consistency of the data was lost.";
                case HPDF_INVALID_FONT_NAME: // 0x102F
                    return "0x102F: A font which has the specified name is not found.";
                case HPDF_INVALID_IMAGE: // 0x1030
                    return "0x1030: Unsupported image format.";
                case HPDF_INVALID_JPEG_DATA: // 0x1031
                    return "0x1031: Unsupported image format.";
                case HPDF_INVALID_N_DATA: // 0x1032
                    return "0x1032: Cannot read a postscript-name from an afm file.";
                case HPDF_INVALID_OBJECT: // 0x1033
                    return "0x1033: 1. An invalid object is set. 2. Internal error. The consistency of the data was lost.";
                case HPDF_INVALID_OBJ_ID: // 0x1034
                    return "0x1034: Internal error. The consistency of the data was lost.";
                case HPDF_INVALID_OPERATION: // 0x1035
                    return "0x1035: 1. Invoked HPDF_Image_SetColorMask() against the image-object which was set a mask-image.";
                case HPDF_INVALID_OUTLINE: // 0x1036
                    return "0x1036: An invalid outline-handle was specified.";
                case HPDF_INVALID_PAGE: // 0x1037
                    return "0x1037: An invalid page-handle was specified.";
                case HPDF_INVALID_PAGES: // 0x1038
                    return "0x1038: An invalid pages-handle was specified. (internel error)";
                case HPDF_INVALID_PARAMETER: // 0x1039
                    return "0x1039: An invalid value is set.";
                case HPDF_INVALID_PNG_IMAGE: // 0x103B
                    return "0x103B: Invalid PNG image format.";
                case HPDF_INVALID_STREAM: // 0x103C
                    return "0x103C: Internal error. The consistency of the data was lost.";
                case HPDF_MISSING_FILE_NAME_ENTRY: // 0x103D
                    return "0x103D: Internal error. The \"_FILE_NAME\" entry for delayed loading is missing.";
                case HPDF_INVALID_TTC_FILE: // 0x103F
                    return "0x103F: Invalid .TTC file format.";
                case HPDF_INVALID_TTC_INDEX: // 0x1040
                    return "0x1040: The index parameter was exceed the number of included fonts";
                case HPDF_INVALID_WX_DATA: // 0x1041
                    return "0x1041: Cannot read a width-data from an afm file.";
                case HPDF_ITEM_NOT_FOUND: // 0x1042
                    return "0x1042: Internal error. The consistency of the data was lost.";
                case HPDF_LIBPNG_ERROR: // 0x1043
                    return "0x1043: An error has returned from PNGLIB while loading an image.";
                case HPDF_NAME_INVALID_VALUE: // 0x1044
                    return "0x1044: Internal error. The consistency of the data was lost.";
                case HPDF_NAME_OUT_OF_RANGE: // 0x1045
                    return "0x1045: Internal error. The consistency of the data was lost.";
                case HPDF_PAGES_MISSING_KIDS_ENTRY: // 0x1049
                    return "0x1049: Internal error. The consistency of the data was lost.";
                case HPDF_PAGE_CANNOT_FIND_OBJECT: // 0x104A
                    return "0x104A: Internal error. The consistency of the data was lost.";
                case HPDF_PAGE_CANNOT_GET_ROOT_PAGES: // 0x104B
                    return "0x104B: Internal error. The consistency of the data was lost.";
                case HPDF_PAGE_CANNOT_RESTORE_GSTATE: // 0x104C
                    return "0x104C: There are no graphics-states to be restored.";
                case HPDF_PAGE_CANNOT_SET_PARENT: // 0x104D
                    return "0x104D: Internal error. The consistency of the data was lost.";
                case HPDF_PAGE_FONT_NOT_FOUND: // 0x104E
                    return "0x104E: The current font is not set.";
                case HPDF_PAGE_INVALID_FONT: // 0x104F
                    return "0x104F: An invalid font-handle was spacified.";
                case HPDF_PAGE_INVALID_FONT_SIZE: // 0x1050
                    return "0x1050: An invalid font-size was set.";
                case HPDF_PAGE_INVALID_GMODE: // 0x1051
                    return "0x1051: See Graphics mode.";
                case HPDF_PAGE_INVALID_INDEX: // 0x1052
                    return "0x1052: Internal error. The consistency of the data was lost.";
                case HPDF_PAGE_INVALID_ROTATE_VALUE: // 0x1053
                    return "0x1053: The specified value is not a multiple of 90.";
                case HPDF_PAGE_INVALID_SIZE: // 0x1054
                    return "0x1054: An invalid page-size was set.";
                case HPDF_PAGE_INVALID_XOBJECT: // 0x1055
                    return "0x1055: An invalid image-handle was set.";
                case HPDF_PAGE_OUT_OF_RANGE: // 0x1056
                    return "0x1056: The specified value is out of range.";
                case HPDF_REAL_OUT_OF_RANGE: // 0x1057
                    return "0x1057: The specified value is out of range.";
                case HPDF_STREAM_EOF: // 0x1058
                    return "0x1058: Unexpected EOF marker was detected.";
                case HPDF_STREAM_READLN_CONTINUE: // 0x1059
                    return "0x1059: Internal error. The consistency of the data was lost.";
                case HPDF_STRING_OUT_OF_RANGE: // 0x105B
                    return "0x105B: The length of the specified text is too long.";
                case HPDF_THIS_FUNC_WAS_SKIPPED: // 0x105C
                    return "0x105C: The execution of a function was skipped because of other errors.";
                case HPDF_TTF_CANNOT_EMBEDDING_FONT: // 0x105D
                    return "0x105D: This font cannot be embedded. (restricted by license)";
                case HPDF_TTF_INVALID_CMAP: // 0x105E
                    return "0x105E: Unsupported ttf format. (cannot find unicode cmap.)";
                case HPDF_TTF_INVALID_FOMAT: // 0x105F
                    return "0x105F: Unsupported ttf format.";
                case HPDF_TTF_MISSING_TABLE: // 0x1060
                    return "0x1060: Unsupported ttf format. (cannot find a necessary table)";
                case HPDF_UNSUPPORTED_FONT_TYPE: // 0x1061
                    return "0x1061: Internal error. The consistency of the data was lost.";
                case HPDF_UNSUPPORTED_FUNC: // 0x1062
                    return "0x1062: 1. The library is not configured to use PNGLIB. 2. Internal error. The consistency of the data was lost.";
                case HPDF_UNSUPPORTED_JPEG_FORMAT: // 0x1063
                    return "0x1063: Unsupported Jpeg format.";
                case HPDF_UNSUPPORTED_TYPE1_FONT: // 0x1064
                    return "0x1064: Failed to parse .PFB file.";
                case HPDF_XREF_COUNT_ERR: // 0x1065
                    return "0x1065: Internal error. The consistency of the data was lost.";
                case HPDF_ZLIB_ERROR: // 0x1066
                    return "0x1066: An error has occurred while executing a function of Zlib.";
                case HPDF_INVALID_PAGE_INDEX: // 0x1067
                    return "0x1067: An error returned from Zlib.";
                case HPDF_INVALID_URI: // 0x1068
                    return "0x1068: An invalid URI was set.";
                // case HPDF_PAGELAYOUT_OUT_OF_RANGE: // 0x1069
                //     return "An invalid page-layout was set.";
                // case HPDF_PAGEMODE_OUT_OF_RANGE: // 0x1070
                //     return "An invalid page-mode was set.";
                // case HPDF_PAGENUM_STYLE_OUT_OF_RANGE: // 0x1071
                //     return "An invalid page-num-style was set.";
                case HPDF_ANNOT_INVALID_ICON: // 0x1072
                    return "0x1072: An invalid icon was set.";
                case HPDF_ANNOT_INVALID_BORDER_STYLE: // 0x1073
                    return "0x1073: An invalid border-style was set.";
                case HPDF_PAGE_INVALID_DIRECTION: // 0x1074
                    return "0x1074: An invalid page-direction was set.";
                case HPDF_INVALID_FONT: // 0x1075
                    return "0x1075: An invalid font-handle was specified.";
            }
            return "[" + std::to_string(hpdf_status) + "]";
        }

        static void hpdf_error_callback(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* document_) {
            // http://libharu.sourceforge.net/error_handling.html
            Document& document = * (Document*) document_;
            std::string message = hpdf_error_name(error_no) + " " + hpdf_error_message(error_no) + " / " + hpdf_error_message(detail_no);
            throw Exceptions::Exception("Error in PDF::Document, " + message);
        }

        const std::string _encoding;
        FontManager _font_manager;
        Style _style;
        std::vector<Page> _pages;
        Page* _current_page;
        size_t _current_page_index;
        Link _current_link;
        std::map<std::filesystem::path, HPDF_Image> _images_cache;

    };


} // PDF


#endif // LINKRBRAIN2019__SRC__PDF__DOCUMENT_HPP
