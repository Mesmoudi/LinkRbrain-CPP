#ifndef LINKRBRAIN2019__SRC__TYPES__VARSTRING_HPP
#define LINKRBRAIN2019__SRC__TYPES__VARSTRING_HPP


#include "./Blob.hpp"

#include <string.h>


namespace Types {

    #pragma pack(push, 1)

    template<typename size_t>
    class VarString : public Blob<size_t> {
    public:

        // instanciation
        inline VarString() : Blob<size_t>() {}
        inline VarString(const size_t source_size) : Blob<size_t>(source_size) {}
        inline VarString(const char* source_data) : Blob<size_t>(source_data, strlen(source_data)) {}
        inline VarString(const std::string& source) : Blob<size_t>(source.data(), source.size()) {}
        template <size_t N>
        inline VarString(const char (&source)[N]) : Blob<size_t>(source, (source[N-1]=='\0') ? (N-1) : (N)) {}

        // override some of the parent class operators
        template<typename other_size_t>
        inline VarString<size_t>& operator+=(const VarString<other_size_t>& other) {
            this->_data = realloc(this->_data, this->_size + other._size - 1);
            memcpy(this->_data + this->_size, other._data, other._size);
            this->_size += other._size - 1;
        }
        inline VarString operator+(const VarString& source) const {
            VarString result(this->_size + source._size);
            memcpy(result._data, this->_data, this->_size);
            memcpy(result._data + this->_size, source._data, source._size);
            return result;
        }

        // conversion to other types
        inline operator const std::string() const {
            return std::string((const char*)this->_data, (const std::size_t)this->_size);
        }

        // comparison with other types
        inline const bool operator == (const char* other) const {
            return
                (strlen(other) == this->_size) &&
                (memcmp(this->_data, other, this->_size) == 0);
        }
        inline const bool operator == (const std::string& other) const {
            return
                (other.size() == (std::size_t)this->_size) &&
                (memcmp(this->_data, other.data(), this->_size) == 0);
        }
        template<typename other_size_t>
        inline const bool operator==(const Blob<other_size_t>& other) {
            return (this->_size == other._size) && !memcmp(this->_data, other._data, this->_size);
        }

        inline const bool operator != (const std::string& other) const {
            return
                (other.size() != (std::size_t)this->_size) ||
                (memcmp(this->_data, other.data(), this->_size) != 0);
        }
        template<typename other_size_t>
        inline const bool operator!=(const Blob<other_size_t>& other) {
            return (this->_size != other._size) || memcmp(this->_data, other._data, this->_size);
        }

    };


    typedef VarString<uint8_t>  VarString8;
    typedef VarString<uint16_t> VarString16;
    typedef VarString<uint24_t> VarString24;
    typedef VarString<uint32_t> VarString32;
    typedef VarString<uint64_t> VarString64;


    #pragma pack(pop)

} // Types


#include <ostream>

// representation
template<typename stringsize_t>
std::ostream& operator << (std::ostream& os, const Types::VarString<stringsize_t>& string) {
    return os.write((const char*)string._data, (const size_t)string._size);
}

namespace std {
    // conversion
    template<typename stringsize_t>
    const std::string to_string(const Types::VarString<stringsize_t>& value) {
        return std::string((const char*)value._data, (const size_t)value._size);
    }
    // hash function
    template<typename stringsize_t>
    struct hash<Types::VarString<stringsize_t>> {
        size_t operator()(const Types::VarString<stringsize_t>& string) const {
            size_t hash = 31L;
            const char* data = (const char*) string._data;
            for (size_t i=0; i<string._size; i++) {
                hash = (hash * 54059L) ^ (76963L * *data++);
            }
            return hash;
        }
    };
};


#endif // LINKRBRAIN2019__SRC__TYPES__VARSTRING_HPP
