#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__TYPES__ARRAY3D_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__TYPES__ARRAY3D_HPP


#include "./PointExtrema.hpp"
#include "Exceptions/GenericExceptions.hpp"
#include "Types/NumberNature.hpp"
#include "Conversion/Binary.hpp"
#include "CLI/Display/Style.hpp"

#include <array>
#include <iostream>


namespace Types {

    template <typename T>
    class Array3D {
    public:

        Array3D() : _data(NULL) {}

        template <typename T2>
        Array3D(const PointExtrema<T2>& extrema, const Point<T> resolution, const bool must_allocate=true) {
            _data = NULL;
            set(extrema, resolution, must_allocate);
        }
        ~Array3D() {
            deallocate();
        }

        template <typename T2>
        void set(const PointExtrema<T2>& extrema, const Point<T> resolution, const bool must_allocate=true) {
            _resolution = resolution;
            _extrema = extrema;
            init(must_allocate);
        }

        //

        class Iterator {
        public:
            struct Data {
                size_t index;
                Point<size_t> indices;
                Point<T> coordinates;
                T* value;
            };
            Iterator() : _is_iterable(false) {}
            Iterator(
                const PointExtrema<size_t>& limits,
                const size_t& index,
                const size_t& x_shift,
                const size_t& y_shift,
                const Point<T>& start,
                const Point<T>& resolution,
                T* values
            ) :
                _is_iterable(true),
                _limits(limits),
                _resolution(resolution),
                _values(values),
                _x_shift(x_shift),
                _y_shift(y_shift),
                _start(start),
                _data({
                    .index = index,
                    .indices = limits.min,
                    .coordinates = start,
                    .value = NULL,
                })
            {
                _data.value = _values + _data.index;
            }
            inline Iterator& operator++ () {
                size_t shift = 1;
                // output << "INCREMENT FROM " << _data.coordinates.x << ", " << _data.coordinates.y << ", " << _data.coordinates.z << '\n';
                if (++_data.indices.z <= _limits.max.z) {
                    _data.coordinates.z += _resolution.z;
                } else {
                    // output << "PASSED Z LIMIT" << '\n';
                    _data.coordinates.z = _start.z;
                    _data.indices.z = _limits.min.z;
                    if (++_data.indices.y <= _limits.max.y) {
                        shift = _y_shift;
                        _data.coordinates.y += _resolution.y;
                    } else {
                        // output << "PASSED Y LIMIT" << '\n';
                        _data.indices.y = _limits.min.y;
                        _data.coordinates.y = _start.y;
                        // output << _data.indices.x << '\n';
                        // output << _limits.max.x << '\n';
                        if (++_data.indices.x <= _limits.max.x) {
                            shift = _x_shift;
                            _data.coordinates.x += _resolution.x;
                        } else {
                            _is_iterable = false;
                        }
                    }
                }
                _data.index += shift;
                _data.value += shift;
                return *this;
            }
            inline operator bool() const {
                return _is_iterable;
            }
            inline Data& operator * () {
                return _data;
            }
            inline Data& operator -> () {
                return _data;
            }
            inline Iterator& begin() {
                return *this;
            }
            inline static const bool end() {
                return false;
            }
            //
            inline const PointExtrema<size_t>& get_limits() const {
                return _limits;
            }
        private:
            Data _data;
            size_t _index;
            const PointExtrema<size_t> _limits;
            const Point<T> _resolution;
            const Point<T> _start;
            T* _values;
            double _is_iterable;
            size_t _x_shift;
            size_t _y_shift;
        };

        template <typename T2>
        Iterator restrict_coordinates(PointExtrema<T2> boundaries) const {
            boundaries &= _extrema;
            if (boundaries.have_nan()) {
                return Iterator();
            }
            PointExtrema<size_t> limits = {
                Point<size_t>(
                    round((boundaries.min.x - _x0) / _resolution.x),
                    round((boundaries.min.y - _y0) / _resolution.y),
                    round((boundaries.min.z - _z0) / _resolution.z)
                ),
                Point<size_t>(
                    round((boundaries.max.x - _x0) / _resolution.x),
                    round((boundaries.max.y - _y0) / _resolution.y),
                    round((boundaries.max.z - _z0) / _resolution.z)
                ),
            };
            return Iterator(
                limits,
                compute_index(boundaries.min.x, boundaries.min.y, boundaries.min.z),
                _x_factor - _y_factor * (limits.max.y - limits.min.y) + limits.min.z - limits.max.z,
                _y_factor + limits.min.z - limits.max.z,
                boundaries.min,
                _resolution,
                _data
            );
        }
        Iterator begin() const {
            return {
                {
                    Point<size_t>(0, 0, 0),
                    Point<size_t>(_x_size-1, _y_size-1, _z_size-1)
                },
                0,
                1,
                1,
                {_x0, _y0, _z0},
                _resolution,
                _data
            };
        }
        inline static const bool end() {
            return false;
        }

        //

        const float get_x_factor() const {
            return _x_factor;
        }
        const float get_y_factor() const {
            return _y_factor;
        }
        const size_t& get_size() const {
            return _size;
        }
        const size_t& get_x_size() const {
            return _x_size;
        }
        const size_t& get_y_size() const {
            return _y_size;
        }
        const size_t& get_z_size() const {
            return _z_size;
        }
        const float get_x0() const {
            return _x0;
        }
        const float get_y0() const {
            return _y0;
        }
        const float get_z0() const {
            return _z0;
        }

        void deallocate() {
            if (_data != NULL) {
                free(_data);
                _data = NULL;
            }
        }
        void allocate() {
            deallocate();
            _data = (T*) calloc(_size, sizeof(T));
            if (_data == NULL) {
                except("Could not allocate", _data_size, "bytes for Array3D");
            }
        }

        inline const size_t compute_index(const T& x, const T& y, const T& z) const {
            return make_index(
                round((x - _x0) / _resolution.x),
                round((y - _y0) / _resolution.y),
                round((z - _z0) / _resolution.z)
            );
        }
        inline const size_t make_index(size_t X, size_t Y, size_t Z) const {
            return X * _x_factor + Y * _y_factor + Z;
        }
        inline const std::array<std::pair<size_t, T>, 8> compute_indices(const T& x, const T& y, const T& z) const {
            const T X_ = (x - _x0) / _resolution.x;
            const T Y_ = (y - _y0) / _resolution.y;
            const T Z_ = (z - _z0) / _resolution.z;
            const size_t X = X_;
            const size_t Y = Y_;
            const size_t Z = Z_;
            const T a = X_ - X;
            const T b = Y_ - Y;
            const T c = Z_ - Z;
            const T a_ = static_cast<T>(1.0) - a;
            const T b_ = static_cast<T>(1.0) - b;
            const T c_ = static_cast<T>(1.0) - c;
            const T w0 = a_ * b_ * c_ ;
            const T w1 = a  * b_ * c_ ;
            const T w2 = a  * b  * c_ ;
            const T w3 = a_ * b  * c_ ;
            const T w4 = a_ * b  * c  ;
            const T w5 = a  * b  * c  ;
            const T w6 = a_ * b  * c  ;
            const T w7 = a_ * b_ * c  ;
            const T W = w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7;
            return {{
                {make_index(X  , Y  , Z  ),  w0 / W},
                {make_index(X+1, Y  , Z  ),  w1 / W},
                {make_index(X+1, Y+1, Z  ),  w2 / W},
                {make_index(X  , Y+1, Z  ),  w3 / W},
                {make_index(X  , Y+1, Z+1),  w4 / W},
                {make_index(X+1, Y+1, Z+1),  w5 / W},
                {make_index(X  , Y+1, Z+1),  w6 / W},
                {make_index(X  , Y  , Z+1),  w7 / W},
            }};
        }
        inline const Point<T> compute_point(size_t index) const {
            const T Z = index % _z_size;
            index /= _z_size;
            const T Y = index % _y_size;
            index /= _y_size;
            const T X = index;
            return {
                _x0 + X * _resolution.x,
                _y0 + Y * _resolution.y,
                _z0 + Z * _resolution.z,
            };
        }

        inline const T& get_value_at(const size_t& index) const {
            return _data[index];
        }
        inline const T& get_value(const T& x, const T& y, const T& z) const {
            return _data[compute_index(x, y, z)];
        }
        inline void set_value(const size_t& x, const size_t& y, const size_t& z, const T& value) {
            return _data[compute_index(x, y, z)] = value;
        }

        inline void clear_data() {
            memset(_data, 0, _data_size);
        }
        inline T* get_data() {
            return _data;
        }
        inline const T* get_data() const {
            return _data;
        }
        inline const bool has_data() const {
            return (_data != NULL);
        }

        inline const Point<T>& get_resolution() const {
            return _resolution;
        }
        const PointExtrema<T>& get_extrema() const {
            return _extrema;
        }
        const size_t& get_data_size() const {
            return _data_size;
        }

        //

        enum Dimension : uint8_t {
            X = 0,
            Y = 1,
            Z = 2,
        };
        enum ColorMode : uint8_t {
            Grayscale = 0,
            Heatmap = 1,
            Hue = 2,
        };

        inline void show(const uint8_t dimension, const T& position, const ColorMode color_mode=Heatmap, std::ostream& output=std::cout) const {
            // check parameters
            if (dimension > 2) except("First parameter (dimension) should be between 0 and 2");
            // do the thing
            show((const Dimension) dimension, position, color_mode, output);
        }
        inline void show(const Dimension dimension, const T& position, const ColorMode color_mode=Heatmap, std::ostream& output=std::cout) const {
            // adapt parameters
            PointExtrema<T> window{_extrema};
            window.min.values[dimension] = window.max.values[dimension] = position;
            // find values extrema
            T value_min = std::numeric_limits<T>::max();
            T value_max = std::numeric_limits<T>::min();
            for (const auto& iterator : restrict_coordinates(window)) {
                if (*iterator.value < value_min)  value_min = *iterator.value;
                if (*iterator.value > value_max)  value_max = *iterator.value;
            }
            // show legend
            for (int i=0; i<3; ++i) {
                output << "xyz"[i] << " = ";
                if (i == dimension) {
                    output << position << " (";
                }
                output << _extrema.min.values[i] << " ... " << _extrema.max.values[i];
                if (i == dimension) {
                    output << ')';
                }
                output << '\n';
            }
            output << "resolution = " << _resolution.x << " x " << _resolution.y << " x " << _resolution.z << '\n';
            output << "value = " << value_min << " ... " << value_max << '\n';
            output << '\n';
            // print to screen
            const int dimension_x = (dimension==0) ? 1 : 0;
            const int dimension_y = (dimension==2) ? 1 : 2;
            T coordinates_top[3] = {position, position, position};
            T coordinates_bottom[3] = {position, position, position};
            const int width = (_extrema.max.values[dimension_x] - _extrema.min.values[dimension_x]) / _resolution.values[dimension_x];
            output << CLI::Display::Style().set_background_gray(0).set_foreground_gray(.33) << "╳" << CLI::Display::Style(false);
            if (width > 2) {
                output << std::string(width - 2, ' ');
            }
            output << CLI::Display::Style().set_background_gray(0).set_foreground_gray(.33) << "╳" << CLI::Display::Style(false);
            output << '\n' << CLI::Display::Style(true);
            for (T y=_extrema.max.values[dimension_y]; y>_extrema.min.values[dimension_y]; y-=2.*_resolution.values[dimension_y]) {
                coordinates_top[dimension_y] = y - .5*_resolution.values[dimension_y];
                coordinates_bottom[dimension_y] = y - 1.5*_resolution.values[dimension_y];
                for (T x=_extrema.min.values[dimension_x]; x<_extrema.max.values[dimension_x]; x+=_resolution.values[dimension_x]) {
                    coordinates_top[dimension_x] = coordinates_bottom[dimension_x] = x + .5*_resolution.values[dimension_x];
                    // printf("\e[3%d;4%dm▀", make_color(coordinates_top, value_min, value_max), make_color(coordinates_bottom, value_min, value_max));
                    const T value_top = (get_value(coordinates_top[0], coordinates_top[1], coordinates_top[2]) - value_min) / (value_max - value_min);
                    const T value_bottom = (get_value(coordinates_bottom[0], coordinates_bottom[1], coordinates_bottom[2]) - value_min) / (value_max - value_min);
                    switch (color_mode) {
                        case Grayscale:
                            output << CLI::Display::Style().set_foreground_gray(value_top).set_background_gray(value_bottom);
                            break;
                        case Heatmap:
                            output << CLI::Display::Style().set_foreground_heatmap(value_top).set_background_heatmap(value_bottom);
                            break;
                        case Hue:
                            output << CLI::Display::Style().set_foreground_heatmap(value_top, true).set_background_heatmap(value_bottom, true);
                            break;
                    }
                    output << "▀";
                }
                output << CLI::Display::Style(false) << '\n';
            }
            output << CLI::Display::Style().set_background_gray(0).set_foreground_gray(.33) << "╳" << CLI::Display::Style(false);
            if (width > 2) {
                output << std::string(width - 2, ' ');
            }
            output << CLI::Display::Style().set_background_gray(0).set_foreground_gray(.33) << "╳" << CLI::Display::Style(false);
            output << '\n' << CLI::Display::Style(true);
        }

    private:

        void init(const bool must_allocate) {
            if (_data != NULL) {
                deallocate();
            }
            _x0 = _resolution.x * round(_extrema.min.x / _resolution.x);
            _y0 = _resolution.y * round(_extrema.min.y / _resolution.y);
            _z0 = _resolution.z * round(_extrema.min.z / _resolution.z);
            _x_size = round(_extrema.max.x / _resolution.x) - round(_extrema.min.x / _resolution.x) + 1;
            _y_size = round(_extrema.max.y / _resolution.y) - round(_extrema.min.y / _resolution.y) + 1;
            _z_size = round(_extrema.max.z / _resolution.z) - round(_extrema.min.z / _resolution.z) + 1;
            _x_factor = _y_size * _z_size;
            _y_factor = _z_size;
            _size = _x_size * _y_size * _z_size;
            _data_size = _size * sizeof(T);
            _data = NULL;
            if (must_allocate) {
                allocate();
            }
        }

        size_t _x_size, _y_size, _z_size;
        size_t _size;
        size_t _data_size;
        T* _data;
        Point<T> _resolution;
        T _x0, _y0, _z0;
        T _x_factor, _y_factor;
        PointExtrema<T> _extrema;
    };

} // Types


namespace Conversion::Binary {

    template <typename T>
    void array3d_serialize(std::ostream& buffer, const Types::Array3D<T>& array3d) {
        straight_serialize(buffer, Types::NumberNatureOf<T>);
        straight_serialize(buffer, array3d.get_extrema());
        serialize(buffer, array3d.get_resolution());
        serialize(buffer, array3d.has_data());
        buffer.seekp(256, std::ios_base::cur);
        if (array3d.has_data()) {
            buffer.write((const char*) array3d.get_data(), array3d.get_data_size());
        }
    }

    template <>
    void serialize<Types::Array3D<float>>(std::ostream& buffer, const Types::Array3D<float>& source) {
        array3d_serialize<float>(buffer, source);
    }
    template <>
    void serialize<Types::Array3D<double>>(std::ostream& buffer, const Types::Array3D<double>& source) {
        array3d_serialize<double>(buffer, source);
    }
    template <>
    void serialize<Types::Array3D<long double>>(std::ostream& buffer, const Types::Array3D<long double>& source) {
        array3d_serialize<long double>(buffer, source);
    }

    template <typename T>
    void array3d_parse(std::istream& buffer, Types::Array3D<T>& array3d) {
        Types::NumberNature number_nature = {.type=Types::NumberNature::Other};
        straight_parse(buffer, number_nature);
        if (number_nature != Types::NumberNatureOf<T>) {
            throw Exceptions::BadDataException("Number nature is different in template parameter (" + Types::NumberNatureOf<T>.get_full_name() + ") than in loaded file (" + number_nature.get_full_name() + ").", {});
        }
        // initialization parameters
        Types::Point<T> resolution;
        Types::PointExtrema<T> extrema;
        bool must_allocate;
        straight_parse(buffer, extrema);
        straight_parse(buffer, resolution);
        parse(buffer, must_allocate);
        // initialize
        array3d.set(extrema, resolution, must_allocate);
        buffer.ignore(256);
        buffer.read((char*) array3d.get_data(), array3d.get_data_size());
    }

    template <>
    void parse<Types::Array3D<float>>(std::istream& buffer, Types::Array3D<float>& destination) {
        array3d_parse(buffer, destination);
    }
    template <>
    void parse<Types::Array3D<double>>(std::istream& buffer, Types::Array3D<double>& destination) {
        array3d_parse(buffer, destination);
    }
    template <>
    void parse<Types::Array3D<long double>>(std::istream& buffer, Types::Array3D<long double>& destination) {
        array3d_parse(buffer, destination);
    }
} // Types

#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__TYPES__ARRAY3D_HPP
