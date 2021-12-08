#include <iostream>

#include "Conversion/Base64.hpp"


int main(int argc, char const *argv[]) {
    // const std::string original = "Man is";
    // const std::string encoded = "TWFuIGlz";
    const std::string original = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";
    const std::string encoded = "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";

    std::cout << "\n\n";
    std::cout << original << "\n\n";
    std::cout << encoded << "\n\n";
    std::cout << Conversion::Base64::straight_parse(encoded) << "\n\n";

    const std::string decoded = Conversion::Base64::straight_parse(encoded);
    std::cout << "SIZE: " << decoded.size() << " vs. " << original.size() << '\n';
    std::cout << ((Conversion::Base64::straight_parse(encoded) == original) ? "TEST PASSED!" : "TEST FAILED!") << "\n\n";

    return 0;
}
