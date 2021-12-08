#ifndef LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__ISTREAM_HPP
#define LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__ISTREAM_HPP


#include <istream>

#include "./IStreamBuf.hpp"


namespace Compression::Zlib {

    class IStream : public std::istream {
    public:
        IStream(std::istream & is)
            : std::istream(new IStreamBuf(is.rdbuf()))
        {
            exceptions(std::ios_base::badbit);
        }
        explicit IStream(std::streambuf * sbuf_p)
            : std::istream(new IStreamBuf(sbuf_p))
        {
            exceptions(std::ios_base::badbit);
        }
        virtual ~IStream()
        {
            delete rdbuf();
        }
    }; // class istream

}


#endif // LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__ISTREAM_HPP
