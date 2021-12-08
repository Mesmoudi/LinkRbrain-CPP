#ifndef LINKRBRAIN2019__SRC__TYPES__NUMBERNATURE_HPP
#define LINKRBRAIN2019__SRC__TYPES__NUMBERNATURE_HPP


#include "Types/FixedPoint.hpp"

#include <stdint.h>
#include <string>


namespace Types {

    #pragma pack(push, 1)
    struct NumberNature {
        enum Type : uint8_t {
            Other = 0,
            Integer = 1,
            Floating = 2,
            FixedPoint = 3,
        };

        const Type type;
        const uint8_t size;
        const bool is_signed : 1;
        const uint8_t fixed_integral_bits : 7;
        const uint8_t fixed_fractional_bits : 8;

        const std::string& get_type_name() const {
            static const std::string names[] = {
                "Other",
                "Integer",
                "Floating",
                "FixedPoint",
            };
            return names[type];
        }
        const std::string get_full_name() const {
            switch (type) {
                case Integer:
                    return std::to_string(size) + " bytes " + (is_signed?"":"un") + "signed integer";
                case Floating:
                    return std::to_string(size) + " bytes " + (is_signed?"":"un") + "signed float";
                case FixedPoint:
                    return std::to_string(size) + " bytes " + (is_signed?"":"un") + "signed fixed with " + std::to_string(fixed_fractional_bits) + " fractional bits";
                default:
                    return "unknown";
            }
        }
        const bool operator == (const NumberNature& other) const {
            return memcmp(this, &other, sizeof(*this)) == 0;
        }
    };
    #pragma pack(pop)


    template <typename T>
    static const NumberNature NumberNatureOf = {
        .type = std::is_integral<T>::value ? NumberNature::Type::Integer :
            (std::is_floating_point<T>::value ? NumberNature::Type::Floating :
                NumberNature::Type::Other
            ),
        .size = sizeof(T),
        .is_signed = std::is_signed<T>::value,
        .fixed_integral_bits = 0,
        .fixed_fractional_bits = 0,
    };
    template <size_t INTEGRAL_SIZE, size_t FRACTIONAL_SIZE, typename INTERNAL_TYPE>
    static const NumberNature NumberNatureOf<FixedPoint<INTEGRAL_SIZE, FRACTIONAL_SIZE, INTERNAL_TYPE>> = {
        .type = NumberNature::Type::FixedPoint,
        .size = sizeof(FixedPoint<INTEGRAL_SIZE, FRACTIONAL_SIZE, INTERNAL_TYPE>),
        .is_signed = std::is_signed<INTERNAL_TYPE>::value,
        .fixed_integral_bits = INTEGRAL_SIZE,
        .fixed_fractional_bits = FRACTIONAL_SIZE,
    };


} // Types

#endif // LINKRBRAIN2019__SRC__TYPES__NUMBERNATURE_HPP
