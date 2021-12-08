#include "Types/FixedPoint.hpp"

#include <iostream>
#include <limits>


typedef float floating;
typedef Types::FixedPoint<5, 11, int32_t> Fixed;
const static std::initializer_list<floating> values = {
    1,
    .999,
    0.0078125,
    0.00390625,
    -3.14159265359,
    2.71828182846,
    20.4,
    -12
};


int main(int argc, char const *argv[]) {
    std::cout << sizeof(Fixed) << " bytes\n\n";
    for (const double value : values) {
        Fixed fixed(value);
        std::cout.precision(std::numeric_limits<floating>::max_digits10);
        std::cout << fixed.to<floating>() << '\n';
    }
    std::cout << "COMPARISON" << '\n';
    std::cout << (Types::FixedPoint<2, 8, uint32_t>(1.25) == Types::FixedPoint<2, 15, uint32_t>(1.25)) << '\n';
    std::cout << (Types::FixedPoint<1, 15, int32_t>(1.25) == Types::FixedPoint<2, 15, int32_t>(1.25)) << '\n';
    std::cout << (Types::FixedPoint<1, 15, int32_t>(1.) < Types::FixedPoint<1, 15, int32_t>(1.25)) << '\n';
    return 0;
}
