#ifndef LINKRBRAIN2019__SRC__MAPPED__HEADER_HPP
#define LINKRBRAIN2019__SRC__MAPPED__HEADER_HPP


#include "Types/FixedString.hpp"
#include "./Exceptions.hpp"


namespace Paged {

    struct Header {

        char _prefix[6];
        struct {
            uint8_t main : 4;
            uint8_t release : 4;
            uint8_t minor : 8;
        } _version;
        Types::FixedString<16> _type;
        std::size_t _page_size;

        inline void set(std::size_t page_size, const std::string& type) {
            memcpy(_prefix, "DUPADB", 6);
            _type = type;
            _page_size = page_size;
        }

        inline void check(std::size_t page_size, const std::string& type, const std::string& file_path) const {
            if (memcmp(_prefix, "DUPADB", 6))
                throw BadHeaderException(file_path, "prefix", "DUPADB", std::string(_prefix));
            if (_page_size != page_size)
                throw BadHeaderException(file_path, "page size", page_size, _page_size);
            if (_type != type)
                throw BadHeaderException(file_path, "type", type, std::to_string(_type));
        }

    };

} // Paged

#endif // LINKRBRAIN2019__SRC__MAPPED__HEADER_HPP
