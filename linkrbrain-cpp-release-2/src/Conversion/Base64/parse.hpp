#ifndef CPPP____INCLUDE____CONVERSION__BASE64__PARSE_HPP
#define CPPP____INCLUDE____CONVERSION__BASE64__PARSE_HPP


#include "../helpers.hpp"
#include "../Binary/parse.hpp"

#include <string>
#include <sstream>


namespace Conversion::Base64 {

    // raw parsing

    std::string compute_reversed_alphabet(const std::string& alphabet) {
        std::string result(256, '\0');
        for (size_t i = 0, n = std::min(256UL, alphabet.size()); i < n; i++) {
            result[(unsigned char) alphabet[i]] = i;
        }
        return result;
    }

    const std::string& reverse_alphabet(const std::string& alphabet) {
        static std::unordered_map<std::string, std::string> cache;
        auto it = cache.find(alphabet);
        if (it != cache.end()) {
            return it->second;
        }
        const std::string reversed = compute_reversed_alphabet(alphabet);
        return cache.insert({alphabet, reversed}).first->second;
    }

    void straight_parse(std::istream& buffer, std::string& destination, const std::string& alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/", const char padding = '=') {
        const std::string reversed_alphabet = reverse_alphabet(alphabet);
        unsigned char octet;
        int missing = 0;
        unsigned char c;
        while (buffer.readsome((char*)&c, 1)) {
            if (c == padding) continue;
            const unsigned char sextet = reversed_alphabet[c];
            if (missing >= 2) {
                octet |= sextet >> (6 - missing);
                destination += (char) octet;
            }
            missing += 2;
            octet = sextet << missing;
            missing %= 8;
        }
    }

    // deducted

    const std::string straight_parse(std::istream& buffer, const std::string& alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/", const char padding = '=') {
        std::string result;
        straight_parse(buffer, result, alphabet, padding);
        return result;
    }
    const std::string straight_parse(const std::string& source, const std::string& alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/", const char padding = '=') {
        std::stringstream buffer(source);
        return straight_parse(buffer, alphabet, padding);
    }

    // deducted

    template<typename T, std::enable_if_t<!std::is_arithmetic<T>::value && !std::is_enum<T>::value, int> = 0>
    void parse(std::istream& buffer, T& destination) {
        std::string base64_decoded;
        straight_parse(buffer, base64_decoded);
        Binary::parse(base64_decoded, destination);
    }


    // helpers

    CPPP____CONVERSION____HELPERS____PARSE


} // Conversion::Base64


#endif // CPPP____INCLUDE____CONVERSION__BASE64__PARSE_HPP
