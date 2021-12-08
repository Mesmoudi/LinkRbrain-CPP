#ifndef LINKRBRAIN2019__SRC__TYPES__POINT_HPP
#define LINKRBRAIN2019__SRC__TYPES__POINT_HPP


#include <string.h>
#include <cmath>

#include "Exceptions/GenericExceptions.hpp"


namespace Types {

    #pragma pack(push,1)

    template <typename T>
    struct Point {

        Point() : Point(0, 0, 0, 0) {}
        Point(const T _x, const T _weight=static_cast<T>(1.0))
            : x(_x)
            , y(_x)
            , z(_x)
            , weight(_weight) {}
        Point(const T _x, const T _y, const T _z, const T _weight=static_cast<T>(1.0))
            : x(_x)
            , y(_y)
            , z(_z)
            , weight(_weight) {}
        template <typename T2>
        Point(const T2& source) :
            x(source.x),
            y(source.y),
            z(source.z),
            weight(source.weight) {}

        union {
            struct {
                T x, y, z, weight;
            };
            T values[4];
        };

        inline static const size_t& compute_hash(const T& value) {
            return * (size_t*) (&value);
        }
        const size_t compute_hash() const {
            return
                compute_hash(x) ^
                compute_hash(y) ^
                compute_hash(z) ^
                compute_hash(weight);
        }

        const bool is_located_at(const Point& other) const {
            return (memcmp(this, &other, 3 * sizeof(T)) == 0);
        }

        const T distance_from(const Point& other) const {
            const T dx = other.x - x;
            const T dy = other.y - y;
            const T dz = other.z - z;
            return std::sqrt(dx*dx + dy*dy + dz*dz);
        }

        const bool operator == (const Point& other) const {
            return (memcmp(values, other.values, 4 * sizeof(T)) == 0);
        }
        const bool operator != (const Point& other) const {
            return (memcmp(values, other.values, 4 * sizeof(T)) != 0);
        }

    };

    #pragma pack(pop)


    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Point<T>& point) {
        return (os
            << "<Point x=" << point.x
            << " y=" << point.y
            << " z=" << point.z
            << " weight=" << point.weight<< ">"
        );
    }

} // Types


#include "Conversion/Binary.hpp"


namespace Conversion::Binary {
    template <typename T>
    void parse(std::istream& buffer, Types::Point<T>& destination) {
        straight_parse(buffer, destination);
    }
    template <typename T>
    void serialize(std::ostream& buffer, const Types::Point<T>& source) {
        straight_serialize(buffer, source);
    }
} // Conversion::Binary

namespace Conversion::JSON {
    template <>
    void serialize<Types::Point<double>>(std::ostream& buffer, const Types::Point<double>& source) {
        buffer << "{\"x\":" << source.x << ",\"y\":" << source.y << ",\"z\":" << source.z << ",\"weight\":" << source.weight << '}';
    }
} // Conversion::JSON


#endif // LINKRBRAIN2019__SRC__TYPES__POINT_HPP
