#ifndef LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__STREAMWRAPPER_HPP
#define LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__STREAMWRAPPER_HPP


#include <zlib.h>

#include "./Exception.hpp"


namespace Compression::Zlib {

    class StreamWrapper : public z_stream {
    public:
        StreamWrapper(bool _is_input = true, int _level = Z_DEFAULT_COMPRESSION)
            : is_input(_is_input)
        {
            this->zalloc = Z_NULL;
            this->zfree = Z_NULL;
            this->opaque = Z_NULL;
            int ret;
            if (is_input)
            {
                this->avail_in = 0;
                this->next_in = Z_NULL;
                ret = inflateInit2(this, 15+32);
            }
            else
            {
                ret = deflateInit2(this, _level, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY);
            }
            if (ret != Z_OK) throw Exception(this, ret);
        }
        ~StreamWrapper()
        {
            if (is_input)
            {
                inflateEnd(this);
            }
            else
            {
                deflateEnd(this);
            }
        }
    private:
        bool is_input;
    }; // class StreamWrapper

}


#endif // LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__STREAMWRAPPER_HPP
