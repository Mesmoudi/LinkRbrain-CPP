#ifndef LINKRBRAIN2019__SRC__TYPES__FIXEDPOINT_HPP
#define LINKRBRAIN2019__SRC__TYPES__FIXEDPOINT_HPP


#include <type_traits>

#include <stdint.h>
#include <memory.h>


// See also: https://gist.github.com/dflemstr/294959


namespace Types {

    #pragma pack(push, 1)

    template <size_t INTEGRAL_SIZE, size_t FRACTIONAL_SIZE, typename INTERNAL_TYPE=int32_t>
    struct FixedPoint {
    public:

        // Constructors

        inline FixedPoint() {}
        template <size_t SOURCE_INTEGRAL_SIZE, size_t SOURCE_FRACTIONAL_SIZE, typename SOURCE_INTERNAL_TYPE>
        inline FixedPoint(const FixedPoint<SOURCE_INTEGRAL_SIZE, SOURCE_FRACTIONAL_SIZE, SOURCE_INTERNAL_TYPE>& source)
            : internal((SOURCE_FRACTIONAL_SIZE == FRACTIONAL_SIZE)
                ? source.internal
                : ((SOURCE_FRACTIONAL_SIZE < FRACTIONAL_SIZE)
                    ? (source.internal << (FRACTIONAL_SIZE - SOURCE_FRACTIONAL_SIZE))
                    : (source.internal >> (SOURCE_FRACTIONAL_SIZE - FRACTIONAL_SIZE))
                )
            ) {}
        template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
        inline FixedPoint(const T& source) : internal(source << FRACTIONAL_SIZE) {}
        template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
        inline FixedPoint(const T& source) : internal(source * (T)((INTERNAL_TYPE)1 << (FRACTIONAL_SIZE - 1))) {}

        // Getters

        template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
        inline const T to() const {
            static const T ratio = (uint64_t)1 << (FRACTIONAL_SIZE - 1);
            return (T)internal / ratio;
        }
        template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
        inline const T to() const {
            return (T)internal >> FRACTIONAL_SIZE;
        }
        inline const float to_float() const {
            return to<float>();
        }
        inline const double to_double() const {
            return to<double>();
        }

        // Comparison operators

        #define FIXED_POINT_COMPARISON_OPERATOR(OP) \
            template <size_t OTHER_INTEGRAL_SIZE, size_t OTHER_FRACTIONAL_SIZE, typename OTHER_INTERNAL_TYPE> \
            inline bool operator OP(const FixedPoint<OTHER_INTEGRAL_SIZE, OTHER_FRACTIONAL_SIZE, OTHER_INTERNAL_TYPE>& other) { \
                if (OTHER_FRACTIONAL_SIZE == FRACTIONAL_SIZE) { \
                    return internal OP other.internal; \
                } else if (OTHER_FRACTIONAL_SIZE < FRACTIONAL_SIZE) { \
                    return internal OP (other.internal << (FRACTIONAL_SIZE - OTHER_FRACTIONAL_SIZE)); \
                } else { \
                    return (internal << (OTHER_FRACTIONAL_SIZE - FRACTIONAL_SIZE)) OP other.internal; \
                } \
            }
        FIXED_POINT_COMPARISON_OPERATOR(==)
        FIXED_POINT_COMPARISON_OPERATOR(!=)
        FIXED_POINT_COMPARISON_OPERATOR(<)
        FIXED_POINT_COMPARISON_OPERATOR(<=)
        FIXED_POINT_COMPARISON_OPERATOR(>)
        FIXED_POINT_COMPARISON_OPERATOR(>=)
        #undef FIXED_POINT_COMPARISON_OPERATOR

        // Unary operators

        inline bool operator !() const { return !internal; }
        inline FixedPoint& operator ++() { internal += one; return *this; }
        inline FixedPoint& operator --() { internal -= one; return *this; }

        // Modification operators

        template <size_t OTHER_INTEGRAL_SIZE, size_t OTHER_FRACTIONAL_SIZE, typename OTHER_INTERNAL_TYPE>
        inline FixedPoint& operator +=(const FixedPoint<OTHER_INTEGRAL_SIZE, OTHER_FRACTIONAL_SIZE, OTHER_INTERNAL_TYPE>& other) {
            if (OTHER_FRACTIONAL_SIZE == FRACTIONAL_SIZE) {
                internal += other.internal;
            } else if (OTHER_FRACTIONAL_SIZE < FRACTIONAL_SIZE) {
                internal += (other.internal << (FRACTIONAL_SIZE - OTHER_FRACTIONAL_SIZE));
            } else {
                internal += (other.internal >> (OTHER_FRACTIONAL_SIZE - FRACTIONAL_SIZE));
            }
            return *this;
        }
        template <size_t OTHER_INTEGRAL_SIZE, size_t OTHER_FRACTIONAL_SIZE, typename OTHER_INTERNAL_TYPE>
        inline FixedPoint& operator -=(const FixedPoint<OTHER_INTEGRAL_SIZE, OTHER_FRACTIONAL_SIZE, OTHER_INTERNAL_TYPE>& other) {
            if (OTHER_FRACTIONAL_SIZE == FRACTIONAL_SIZE) {
                internal -= other.internal;
            } else if (OTHER_FRACTIONAL_SIZE < FRACTIONAL_SIZE) {
                internal -= (other.internal << (FRACTIONAL_SIZE - OTHER_FRACTIONAL_SIZE));
            } else {
                internal -= (other.internal >> (OTHER_FRACTIONAL_SIZE - FRACTIONAL_SIZE));
            }
            return *this;
        }

        // Binary operators

        template <size_t OTHER_INTEGRAL_SIZE, size_t OTHER_FRACTIONAL_SIZE, typename OTHER_INTERNAL_TYPE>
        inline FixedPoint& operator +(const FixedPoint<OTHER_INTEGRAL_SIZE, OTHER_FRACTIONAL_SIZE, OTHER_INTERNAL_TYPE>& other) {
            FixedPoint result(*this);
            result += other;
            return result;
        }

        // Values

        static const INTERNAL_TYPE one = (1 << (FRACTIONAL_SIZE - 1));
        INTERNAL_TYPE internal : (INTEGRAL_SIZE + FRACTIONAL_SIZE);

    };


    #pragma pack(pop)

} // Types

#endif // LINKRBRAIN2019__SRC__TYPES__FIXEDPOINT_HPP
