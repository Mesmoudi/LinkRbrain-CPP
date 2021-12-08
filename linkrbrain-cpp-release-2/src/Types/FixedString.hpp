#ifndef LINKRBRAIN2019__SRC__TYPES__FIXEDSTRING_HPP
#define LINKRBRAIN2019__SRC__TYPES__FIXEDSTRING_HPP


#include <string.h>
#include <string>


namespace Types {

    #pragma pack(push, 1)

    template<size_t _size>
    struct FixedString {

        // internal information
        char _data[_size];


        // instanciation
        inline FixedString() {
            memset(_data, 0, _size);
        }
        inline FixedString(const std::string& source) {
            strncpy(_data, source.data(), _size);
        }
        inline FixedString(const char* source_data) {
            strncpy(_data, source_data, _size);
        }

        // affectation
        template<size_t source_size>
        inline const FixedString<_size>& operator = (const FixedString<source_size>& source) {
            memcpy(_data, source._data, std::min(source_size, _size));
            return *this;
        }
        inline const FixedString<_size>& operator = (const FixedString<_size>& source) {
            memcpy(_data, source._data, _size);
            return *this;
        }
        inline const FixedString<_size>& operator = (const char* source) {
            strncpy(_data, source, _size);
            return *this;
        }
        inline const FixedString<_size>& operator = (const std::string& source) {
            memcpy(_data, source.c_str(), std::min((std::size_t)_size, source.size() + 1));
            return *this;
        }

        // conversions
        inline operator std::string() const {
            return std::string(_data, strnlen(_data, _size));
        }
        
        // comparison with strings of the same size
        inline const bool operator < (const FixedString<_size>& other) const {
            return memcmp(_data, other._data, _size) < 0;
        }
        inline const bool operator <= (const FixedString<_size>& other) const {
            return memcmp(_data, other._data, _size) <= 0;
        }
        inline const bool operator == (const FixedString<_size>& other) const {
            return memcmp(_data, other._data, _size) == 0;
        }
        inline const bool operator >= (const FixedString<_size>& other) const {
            return memcmp(_data, other._data, _size) >= 0;
        }
        inline const bool operator > (const FixedString<_size>& other) const {
            return memcmp(_data, other._data, _size) > 0;
        }
        // comparison with strings of another size
        template<std::size_t size2>
        inline const bool operator < (const FixedString<size2>& other) const {
            return memcmp(_data, other._data, std::min(_size, size2)) < 0;
        }
        template<std::size_t size2>
        inline const bool operator <= (const FixedString<size2>& other) const {
            return memcmp(_data, other._data, std::min(_size, size2)) <= 0;
        }
        template<std::size_t size2>
        inline const bool operator == (const FixedString<size2>& other) const {
            return memcmp(_data, other._data, std::min(_size, size2)) == 0;
        }
        template<std::size_t size2>
        inline const bool operator >= (const FixedString<size2>& other) const {
            return memcmp(_data, other._data, std::min(_size, size2)) >= 0;
        }
        template<std::size_t size2>
        inline const bool operator > (const FixedString<size2>& other) const {
            return memcmp(_data, other._data, std::min(_size, size2)) > 0;
        }
        // comparison with other types
        inline const bool operator == (const char* other) const {
            return strncmp(_data, other, _size) == 0;
        }
        inline const bool operator != (const char* other) const {
            return strncmp(_data, other, _size) != 0;
        }
        inline const bool operator == (const std::string& other) const {
            return strncmp(_data, other.c_str(), (std::size_t)_size) == 0;
        }
        inline const bool operator != (const std::string& other) const {
            return strncmp(_data, other.c_str(), (std::size_t)_size) != 0;
        }

        // other
        inline const std::size_t size() const {
            return strnlen(_data, _size);
        }

    };

    #pragma pack(pop)

} // Types


#include <ostream>

// representation
template<size_t string_size>
std::ostream& operator << (std::ostream& os, const Types::FixedString<string_size>& string) {
    os.write(string._data, strnlen(string._data, string_size));
    return os;
}

namespace std {
    // conversion
    template<size_t string_size>
    const std::string to_string(const Types::FixedString<string_size>& string) {
        return std::string(string._data, strnlen(string._data, string_size));
    }
    // hash function
    template<size_t string_size>
    struct hash<Types::FixedString<string_size>> {
        size_t operator()(const Types::FixedString<string_size>& string) const {
            size_t hash = 31L;
            const char* data = (const char*) string._data;
            for (size_t i=0; i<string_size; i++) {
                hash = (hash * 54059L) ^ (76963L * *data++);
            }
            return hash;
        }
    };
};


#endif // LINKRBRAIN2019__SRC__TYPES__FIXEDSTRING_HPP
