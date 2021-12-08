#include "Types/NumberNature.hpp"
#include "Types/FixedPoint.hpp"

#include <iostream>
#include <stdint.h>
#include <vector>


// typedef float Number;
// typedef double Number;
// typedef long double Number;
// typedef Types::FixedPoint<4, 12, uint16_t> Number;
// typedef Types::FixedPoint<8, 16, int32_t> Number;
// typedef int8_t Number;
typedef uint64_t Number;


int main(int argc, char const *argv[]) {
    std::cout << "NumberNature size: " << sizeof(Types::NumberNature) << '\n';
    std::cout << '\n';
    std::cout << "Type designation: " << Types::NumberNatureOf<Number>.get_type_name() << '\n';
    std::cout << "Is signed: " << std::boolalpha << Types::NumberNatureOf<Number>.is_signed<< '\n';
    std::cout << "Bytes: " << (int) Types::NumberNatureOf<Number>.size << '\n';
    std::cout << "Integral bits: " << (int) Types::NumberNatureOf<Number>.fixed_integral_bits << '\n';
    std::cout << "Fractional bits: " << (int) Types::NumberNatureOf<Number>.fixed_fractional_bits << '\n';
    return 0;
}
