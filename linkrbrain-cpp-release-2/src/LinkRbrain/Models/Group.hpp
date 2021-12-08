#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__GROUP_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__GROUP_HPP


#include <stdint.h>

#include <string>
#include <vector>
#include <cmath>
#include <unordered_map>

#include "Types/Point.hpp"
#include "Types/PointExtrema.hpp"
#include "Types/Entity.hpp"

#include "Conversion/Binary.hpp"


namespace LinkRbrain::Correlation {
    template <typename T>
    class Correlator;
}


namespace LinkRbrain::Models {

    template <typename T>
    class Group : public Types::Entity {
    public:

        Group(const std::string& label="") :
            Types::Entity(label) {}

        void get_points(const std::vector<Types::Point<T>>& points) {
            _points = points;
        }
        std::vector<Types::Point<T>>& get_points() {
            return _points;
        }
        const std::vector<Types::Point<T>>& get_points() const {
            return _points;
        }
        Types::Point<T>& add_point(const Types::Point<T>& point) {
            _points.push_back(point);
            return _points.back();
        }

        inline void integrate_point(const Types::Point<T>& point) {
            if (point.weight == 0) {
                return;
            }
            for (Types::Point<T>& existing_point : _points) {
                if (existing_point.is_located_at(point)) {
                    existing_point.weight += point.weight;
                    return;
                }
            }
            add_point(point);
        }
        template <typename Container>
        void integrate_points(const Container& points) {
            for (const Types::Point<T>& point : points) {
                integrate_point(point);
            }
        }
        Types::Point<T>& add_point(const T x, const T y, const T z, const T weight=1.) {
            _points.push_back({x, y, z, weight});
            return _points.back();
        }
        Types::Point<T>& upsert_point(const Types::Point<T>& point) {
            for (Types::Point<T>& compared_point : _points) {
                if (compared_point.is_located_at(point)) {
                    compared_point.weight += point.weight;
                    return compared_point;
                }
            }
            return add_point(point);
        }

        const size_t compute_hash() const {
            size_t hash = 0;
            for (const auto& point : _points) {
                hash ^= point.compute_hash();
            }
            return hash;
        }

    private:

        std::vector<Types::Point<T>> _points;

    };

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Group<T>& group) {
        os << "<Group label=" << group.get_label() << " points=";
        for (const Types::Point<T>& point : group.get_points()) {
            os << "\n        " << point;
        }
        return os << ">";
    }


} // LinkRbrain::Models


namespace Conversion::Binary {

    template <typename T>
    void group_parse(std::istream& buffer, LinkRbrain::Models::Group<T>& destination) {
        parse(buffer, (Types::Entity&) destination);
        size_t size;
        parse(buffer, size);
        std::vector<Types::Point<T>>& points = destination.get_points();
        points.resize(size);
        buffer.read((char*) &points[0], size * sizeof(points[0]));
    }

    template <>
    void parse<LinkRbrain::Models::Group<float>>(std::istream& buffer, LinkRbrain::Models::Group<float>& destination) {
        group_parse(buffer, destination);
    }
    template <>
    void parse<LinkRbrain::Models::Group<double>>(std::istream& buffer, LinkRbrain::Models::Group<double>& destination) {
        group_parse(buffer, destination);
    }
    template <>
    void parse<LinkRbrain::Models::Group<long double>>(std::istream& buffer, LinkRbrain::Models::Group<long double>& destination) {
        group_parse(buffer, destination);
    }



    template <typename T>
    void group_serialize(std::ostream& buffer, const LinkRbrain::Models::Group<T>& source) {
        serialize(buffer, (Types::Entity&) source);
        const std::vector<Types::Point<T>>& points = source.get_points();
        serialize(buffer, points.size());
        buffer.write((char*) &points[0], points.size() * sizeof(points[0]));
    }

    template <>
    void serialize<LinkRbrain::Models::Group<float>>(std::ostream& buffer, const LinkRbrain::Models::Group<float>& source) {
        group_serialize(buffer, source);
    }
    template <>
    void serialize<LinkRbrain::Models::Group<double>>(std::ostream& buffer, const LinkRbrain::Models::Group<double>& source) {
        group_serialize(buffer, source);
    }
    template <>
    void serialize<LinkRbrain::Models::Group<long double>>(std::ostream& buffer, const LinkRbrain::Models::Group<long double>& source) {
        group_serialize(buffer, source);
    }

} // Conversion::Binary


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__GROUP_HPP
