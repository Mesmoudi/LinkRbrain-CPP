#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__PDF__SLICESCACHE_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__PDF__SLICESCACHE_HPP


#include "Exceptions/GenericExceptions.hpp"

#include <map>
#include <filesystem>

#include <cairo/cairo.h>


namespace LinkRbrain::PDF {


    class SlicesCache {
    public:

        SlicesCache(const std::filesystem::path& base_path = "var/www/images/brain") {
            set_base_path(base_path);
        }

        ~SlicesCache() {
            for (auto& [coordinates, cairo_surface] : _cairo_slices_cache) {
                cairo_surface_destroy(cairo_surface);
            }
        }

        //

        void set_base_path(const std::filesystem::path& base_path) {
            _base_path = base_path;
        }

        //

        const std::string& get_cached_png_slice(const std::string& coordinates, const int position) {
            const std::pair<std::string, int> key = {coordinates, position};
            // try to retrieve from cache
            const auto& it = _png_slice_cache.find(key);
            if (it != _png_slice_cache.end()) {
                return it->second;
            }
            // otherwise, compute & cache
            const std::string png_slice = get_png_slice(coordinates, position);
            return (_png_slice_cache[key] = png_slice);
        }

        const int get_slice_offset(const char coordinate) {
            switch (coordinate) {
                case 'x': return  90;
                case 'y': return 126;
                case 'z': return  72;
            }
            return 0;
        }
        const int get_slice_size(const char coordinate) {
            switch (coordinate) {
                case 'x': return  91;
                case 'y': return 109;
                case 'z': return  91;
            }
            return 0;
        }
        const std::string get_png_slice(const std::string& coordinates, const int position) {
            // compute width, height and offset
            const int width = get_slice_size(coordinates[0]);
            const int height = get_slice_size(coordinates[1]);
            const int offset = (position + get_slice_offset(coordinates[2])) / 2 * height;
            // crop image
            cairo_surface_t* cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
            cairo_t* cairo_context = cairo_create(cairo_surface);
            cairo_set_source_surface(cairo_context, get_cairo_slices(coordinates), 0, -offset);
            cairo_paint(cairo_context);
            // convert to PNG
            std::string png_buffer;
            cairo_surface_write_to_png_stream(cairo_surface, [] (void* png_buffer_, const unsigned char* data, unsigned int size) {
                std::string& png_buffer = * (std::string*) png_buffer_;
                png_buffer.append((const char*) data, size);
                return CAIRO_STATUS_SUCCESS;
            }, &png_buffer);
            // destroy things
            cairo_destroy(cairo_context);
            cairo_surface_destroy(cairo_surface);
            // the end!
            return png_buffer;
        }

        cairo_surface_t* get_cairo_slices(const std::string& coordinates) {
            // try to retrieve from cache
            auto it = _cairo_slices_cache.find(coordinates);
            if (it != _cairo_slices_cache.end()) {
                return it->second;
            }
            // load image
            const std::filesystem::path path = _base_path / (coordinates + ".png");
            cairo_surface_t* cairo_surface = cairo_image_surface_create_from_png(path.c_str());
            const cairo_status_t cairo_status = cairo_surface_status(cairo_surface);
            if (cairo_status != CAIRO_STATUS_SUCCESS) {
                throw Exceptions::NotFoundException("Cairo cannot open file `" + path.native() + "` in LinkRbrain::PDF::SlicesCache: " + cairo_status_to_string(cairo_status));
            }
            // do caching, return what was asked
            _cairo_slices_cache.insert({coordinates, cairo_surface});
            return cairo_surface;
        }

    private:

        std::filesystem::path _base_path;
        std::map<std::string, cairo_surface_t*> _cairo_slices_cache;
        std::map<std::pair<std::string, int>, std::string> _png_slice_cache;

    };

} // LinkRbrain::PDF


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__PDF__SLICESCACHE_HPP
