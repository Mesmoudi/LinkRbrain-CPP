#include "Conversion/Binary.hpp"

#include <iostream>


int main(int argc, char const *argv[]) {
    Types::Variant test;
    test["key"] = "value";
    test["array14"] = {1,2,3,4};
    test["booleans"] = {true, false};
    test["deep"]["into"]["the"] = "map";
    test["pi"] = 3.14;
    std::cout << test << '\n';
    const std::string serialized = Conversion::Binary::serialize(test);
    Types::Variant test2;
    Conversion::Binary::parse(serialized, test2);
    std::cout << test2 << '\n';
    return 0;
}
