#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__SCORER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__SCORER_HPP


#include "Types/Point.hpp"
#include "Types/PointExtrema.hpp"
#include "Types/Array3D.hpp"

#include <cmath>
#include <string>


namespace LinkRbrain::Scoring {


    class Scorer {
    public:

        enum Mode : int32_t {
            Distance = 0x10,
            Sphere = 0x11,
        };

        Scorer(const Mode& mode=Sphere, const double diameter=10.) {
            set_mode(mode);
            set_diameter(diameter);
        }

        // about the mode

        void set_mode(const Mode& mode) {
            _mode = mode;
        }
        const Mode& get_mode() const {
            return _mode;
        }
        static const std::string& get_mode_name(const Mode& mode) {
            static const std::string mode_names[] = {
                "?",
                "Distance",
                "Sphere",
            };
            return mode_names[mode];
        }
        const std::string& get_mode_name() const {
            return get_mode_name(_mode);
        }

        // options

        inline void set_diameter(const double& diameter) {
            _diameter = diameter;
            _diameter2 = diameter * diameter;
        }
        inline const double& get_diameter() const {
            return _diameter;
        }

        // scoring itself

        template <typename T>
        inline const T score(const Types::Point<T>& p1, const Types::Point<T>& p2) {
            if (_mode & 0x10) {
                const T dx = p1.x - p2.x;
                const T dy = p1.y - p2.y;
                const T dz = p1.z - p2.z;
                const T distance2 = dx*dx + dy*dy + dz*dz;
                if (distance2 > _diameter2) return static_cast<T>(0.);
                const T x = std::sqrt(distance2) / this->_diameter;
                //
                const T w = p1.weight * p2.weight;
                const T w2 = std::sqrt(std::abs(w));
                switch (_mode) {
                    case Distance:
                        return (w<0 ? -w2 : w2) * (static_cast<T>(1.) - x);
                    case Sphere:
                        return (w<0 ? -w2 : w2) * (static_cast<T>(0.5)*x * (x*x - static_cast<T>(3.0)) + static_cast<T>(1.0));
                }
            }
            return static_cast<T>(NAN);
        }

        template <typename T>
        inline const T score(const Types::Point<T>& p1, const std::vector<Types::Point<T>>& points2) {
            T result = static_cast<T>(0);
            for (const Types::Point<T>& p2 : points2) {
                result += score(p1, p2);
            }
            return result;
        }

        template <typename T>
        inline const T score(const std::vector<Types::Point<T>>& points1, const std::vector<Types::Point<T>>& points2) {
            T result = static_cast<T>(0);
            for (const Types::Point<T>& p1 : points1) {
                for (const Types::Point<T>& p2 : points2) {
                    result += score(p1, p2);
                }
            }
            return result;
        }

        template <typename T>
        inline const T autoscore(const std::vector<Types::Point<T>>& points) {
            T result = static_cast<T>(0);
            for (size_t i=0, n=points.size(); i<n; ++i) {
                const Types::Point<T>& p1 = points[i];
                const T increment = score(p1, p1);
                if (!isnan(increment)) {
                    result += increment;
                }
                for (size_t j=i+1; j<n; ++j) {
                    const T increment = static_cast<T>(2.0) * score(p1, points[j]);
                    if (!isnan(increment)) {
                        result += increment;
                    }
                }
            }
            return result;
        }

        // projection

        template <typename T>
        void inflate(Types::PointExtrema<T>& extrema) {
            switch (_mode) {
                case Distance:
                case Sphere:
                    extrema.inflate_dimensions(_diameter);
            }
        }

        template <typename T>
        inline void project(Types::Array3D<T>& densitymap, const Types::Point<T>& point) {
            Types::PointExtrema<T> window(point);
            window.inflate_dimensions(_diameter);
            if (_mode & 0x10) {
                for (auto& iterator : densitymap.restrict_coordinates(window)) {
                    *iterator.value += score(iterator.coordinates, point);
                }
            }
        }

        template <typename T>
        const T compute_overall_score(const std::vector<T>& scores) const {
            switch (scores.size()) {
                case 0:
                    return NAN;
                case 1:
                    return scores[0];
                default: {
                    T sum = static_cast<T>(0);
                    for (const T& score : scores) {
                        sum += score * score;
                    }
                    return std::sqrt(sum);
                }
            }
        }

    private:

        Mode _mode;
        double _diameter;
        double _diameter2;

    };


} // LinkRbrain::Scoring


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__SCORER_HPP
