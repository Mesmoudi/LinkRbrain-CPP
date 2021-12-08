#ifndef LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__OSTREAM_HPP
#define LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__OSTREAM_HPP


#include <ostream>

#include "./OStreamBuf.hpp"


namespace Compression::Zlib {

    class OStream : public std::ostream {
    public:

        OStream(std::ostream& os, std::size_t _buff_size = OStreamBuf::default_buff_size, int _level = Z_DEFAULT_COMPRESSION)
            : std::ostream(new OStreamBuf(os.rdbuf(), _buff_size, _level))
        {
            exceptions(std::ios_base::badbit);
        }
        explicit OStream(std::streambuf * sbuf_p) : std::ostream(new OStreamBuf(sbuf_p)) {
            exceptions(std::ios_base::badbit);
        }
        virtual ~OStream() {
            delete rdbuf();
        }

    }; // class ostream

}


#endif // LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__OSTREAM_HPP
