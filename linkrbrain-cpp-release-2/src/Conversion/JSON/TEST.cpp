#include "Conversion/JSON.hpp"

#include <iostream>


int main(int argc, char const *argv[]) {
    Types::Variant test;
    Conversion::JSON::parse(R""""(
        {"key":"value", "array14":[1,2,3,4], "booleans":[true,false], "deep":{"into":{"the":"map"}, "pi":3.14}}
    )"""", test);
    std::cout << test << '\n';
    Conversion::JSON::serialize(std::cout, test);
    std::cout << '\n';
    return 0;
}
