
#include <iostream>
#include "Conversion/YAML.hpp"


int main(int argc, char const *argv[]) {
    Types::Variant test;
    Conversion::YAML::parse(R""""(
        # - a
        # - b
        key: value
        array14: [1,2,3,4]
        booleans:
            - true
            - "false"
        deep:
            into: {the: map}
        pi: 3.14
        folded: |
            this is a
            multiline test
        literal: >
            this is another
            multiline test
    )"""", test);
    std::cout << test << '\n';
    // Conversion::YAML::serialize(std::cout, test);
    std::cout << '\n';
    return 0;
}
