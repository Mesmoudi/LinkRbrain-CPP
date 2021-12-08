#include <iostream>

#include "Types/FixedString.hpp"
#include "Types/VarString.hpp"


int main(int argc, char const *argv[]) {
    Types::FixedString<8> s1("Ceci est un test.");
    Types::VarString s2("Ceci est un test.");
    std::cout << '`' << s1 << "`\n";
    std::cout << '`' << s2 << "`\n";
    return 0;
}
