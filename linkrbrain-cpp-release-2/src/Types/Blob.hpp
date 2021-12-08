#ifndef LINKRBRAIN2019__SRC__TYPES__BLOB_HPP
#define LINKRBRAIN2019__SRC__TYPES__BLOB_HPP


#include "./Integers.hpp"


namespace Types {

    #pragma pack(push, 1)

    template<typename size_t>
    struct Blob {

        // internal informations
        size_t _size;
        void* _data;


        // instanciation
        inline Blob() : _size(0), _data(NULL) {}
        inline Blob(const size_t source_size) : _size(source_size), _data(malloc(_size)) {}
        inline Blob(const void* source_data, const size_t source_size) : _size(source_size), _data(malloc(_size)) {
            memcpy(_data, source_data, _size);
        }
        inline Blob(const Blob& source) : _size(source._size), _data(malloc(_size)) {
            memcpy(_data, source._data, _size);
        }
        // deinstanciation
        inline ~Blob() {
            if (_size && _data) {
                free(_data);
            }
        }

        // test
        inline operator bool() const {
            return _size && _data;
        }

        // comparison
        template<typename other_size_t>
        inline const bool operator==(const Blob<other_size_t>& other) {
            return (_size == other._size) && !memcmp(_data, other._data, _size);
        }

        // operations
        inline Blob& operator+=(const Blob& source) {
            _data = realloc(_data, _size + source._size);
            memcpy(_data + _size, source._data, source._size);
            _size += source._size;
        }
        inline Blob operator+(const Blob& source) const {
            Blob result(_size + source._size);
            memcpy(result._data, _data, _size);
            memcpy(result._data + _size, source._data, source._size);
            return result;
        }

        // put data
        inline void load(const size_t source_size, const void* source_data) {
            if (_data && _size) {
                free(_data);
            }
            _data = malloc(source_size);
            memcpy(_data, source_data, source_size);
            _size = source_size;
        }

        // access size & data
        inline const size_t size() const {
            return _size;
        }
        inline const void* data() const {
            return _data;
        }

    };


    typedef Blob<uint8_t> Blob8;
    typedef Blob<uint16_t> Blob16;
    typedef Blob<uint24_t> Blob24;
    typedef Blob<uint32_t> Blob32;
    typedef Blob<uint64_t> Blob64;

    #pragma pack(pop)

} // Types


#define is_blob8(value_t) (::std::is_base_of<Types::Blob8, value_t>::value)
#define is_blob16(value_t) (::std::is_base_of<Types::Blob16, value_t>::value)
#define is_blob24(value_t) (::std::is_base_of<Types::Blob24, value_t>::value)
#define is_blob32(value_t) (::std::is_base_of<Types::Blob32, value_t>::value)
#define is_blob64(value_t) (::std::is_base_of<Types::Blob64, value_t>::value)

#define is_blob(value_t) ( \
    is_blob8(value_t) || \
    is_blob16(value_t) || \
    is_blob24(value_t) || \
    is_blob32(value_t) || \
    is_blob64(value_t) \
)


#endif // LINKRBRAIN2019__SRC__TYPES__BLOB_HPP
