#include "Types/DateTime.hpp"
#include "Types/Variant.hpp"


int main(int argc, char const *argv[]) {
    std::cout << sizeof(time_t) << '\n';
    std::cout << sizeof(Types::Variant) << '\n';
    std::cout << sizeof(Types::DateTime) << '\n';

    Types::DateTime programming_time(2019, 10, 17, 13, 02, 11, 12345);
    std::cout << programming_time << '\n';
    std::cout.precision(std::numeric_limits<double>::max_digits10 - 1);
    std::cout << programming_time.get_timestamp() << '\n';
    std::cout << Types::DateTime(1571317331.012345) << '\n';
    std::cout << Types::DateTime(1571317331, 12345) << '\n';
    // std::cout << Types::DateTime(1571312466) << '\n';
    return 0;
}
