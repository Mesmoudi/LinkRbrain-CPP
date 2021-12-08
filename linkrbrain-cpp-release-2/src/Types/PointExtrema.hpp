#ifndef LINKRBRAIN2019__SRC__TYPES__EXTREMA_HPP
#define LINKRBRAIN2019__SRC__TYPES__EXTREMA_HPP


#include "./Point.hpp"

#include <limits>


namespace Types {

    #pragma pack(push,1)

    template <typename T>
    struct PointExtrema {

        Point<T> min, max;

        template <typename T2>
        PointExtrema(Point<T2> _center) :
            PointExtrema(_center, _center) {}
        template <typename T2>
        PointExtrema(Point<T2> _min, Point<T2> _max) :
            min(_min),
            max(_max) {}
        PointExtrema() :
            min(
                std::numeric_limits<T>::max(),
                std::numeric_limits<T>::max(),
                std::numeric_limits<T>::max(),
                std::numeric_limits<T>::max()
            ),
            max(
                std::numeric_limits<T>::min(),
                std::numeric_limits<T>::min(),
                std::numeric_limits<T>::min(),
                -std::numeric_limits<T>::min()
            ) {}
        template <typename T2>
        PointExtrema(const PointExtrema<T2>& source) :
            min(source.min), max(source.max) {}
        template <typename T2>
        void integrate(const Point<T2>& point) {
            if (point.x < min.x) min.x = point.x;
            if (point.x > max.x) max.x = point.x;
            if (point.y < min.y) min.y = point.y;
            if (point.y > max.y) max.y = point.y;
            if (point.z < min.z) min.z = point.z;
            if (point.z > max.z) max.z = point.z;
            if (point.weight < min.weight) min.weight = point.weight;
            if (point.weight > max.weight) max.weight = point.weight;
        }
        void integrate(const Point<T>& point) {
            if (point.x < min.x) min.x = point.x;
            if (point.x > max.x) max.x = point.x;
            if (point.y < min.y) min.y = point.y;
            if (point.y > max.y) max.y = point.y;
            if (point.z < min.z) min.z = point.z;
            if (point.z > max.z) max.z = point.z;
            if (point.weight < min.weight) min.weight = point.weight;
            if (point.weight > max.weight) max.weight = point.weight;
        }
        template <typename T2>
        void inflate_dimensions(const T2& value) {
            min.x -= value;
            min.y -= value;
            min.z -= value;
            max.x += value;
            max.y += value;
            max.z += value;
        }

        const bool have_nan() const {
            return (
                std::isnan(min.x) || std::isnan(min.y) || std::isnan(min.z) ||
                std::isnan(max.x) || std::isnan(max.y) || std::isnan(max.z)
            );
        }

        PointExtrema<T>& operator &= (const PointExtrema<T>& other) {
            // x
            if (min.x > other.max.x || max.x < other.min.x) {
                min.x = NAN;
                max.x = NAN;
            }
            if (min.x < other.min.x) min.x = other.min.x;
            if (max.x > other.max.x) max.x = other.max.x;
            // y
            if (min.y > other.max.y || max.y < other.min.y) {
                min.y = NAN;
                max.y = NAN;
            }
            if (min.y < other.min.y) min.y = other.min.y;
            if (max.y > other.max.y) max.y = other.max.y;
            // z
            if (min.z > other.max.z || max.z < other.min.z) {
                min.z = NAN;
                max.z = NAN;
            }
            if (min.z < other.min.z) min.z = other.min.z;
            if (max.z > other.max.z) max.z = other.max.z;
            return *this;
        }

    };

    #pragma pack(pop)

} // Types


#endif // LINKRBRAIN2019__SRC__TYPES__EXTREMA_HPP
