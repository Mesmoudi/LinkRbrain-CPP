#ifndef LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__OSTREAMBUF_HPP
#define LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__OSTREAMBUF_HPP


#include <cassert>

#include "./StreamWrapper.hpp"


namespace Compression::Zlib {

    class OStreamBuf : public std::streambuf {
    public:

        OStreamBuf(std::streambuf* _sbuf_p,
                   std::size_t _buff_size = default_buff_size, int _level = Z_DEFAULT_COMPRESSION)
            : sbuf_p(_sbuf_p),
              zstrm_p(new StreamWrapper(false, _level)),
              buff_size(_buff_size)
        {
            assert(sbuf_p);
            in_buff = new char [buff_size];
            out_buff = new char [buff_size];
            setp(in_buff, in_buff + buff_size);
        }

        OStreamBuf(const OStreamBuf &) = delete;
        OStreamBuf(OStreamBuf &&) = default;
        OStreamBuf & operator = (const OStreamBuf &) = delete;
        OStreamBuf & operator = (OStreamBuf &&) = default;

        int deflate_loop(int flush) {
            int ret;
            std::streamsize sz;
            do {
                zstrm_p->next_out = reinterpret_cast< decltype(zstrm_p->next_out) >(out_buff);
                zstrm_p->avail_out = buff_size;
                ret = deflate(zstrm_p, flush);
                if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) throw Exception(zstrm_p, ret);
                sz = sbuf_p->sputn(out_buff, reinterpret_cast< decltype(out_buff) >(zstrm_p->next_out) - out_buff);
                if (sz != reinterpret_cast< decltype(out_buff) >(zstrm_p->next_out) - out_buff) {
                    // there was an error in the sink stream
                    return -1;
                }
            } while (ret != Z_STREAM_END && ret != Z_BUF_ERROR && sz != 0);
            return 0;
        }

        virtual ~OStreamBuf() {
            // flush the zlib stream
            //
            // NOTE: Errors here (sync() return value not 0) are ignored, because we
            // cannot throw in a destructor. This mirrors the behaviour of
            // std::basic_filebuf::~basic_filebuf(). To see an exception on error,
            // close the ofstream with an explicit call to close(), and do not rely
            // on the implicit call in the destructor.
            //
            sync();
            delete [] in_buff;
            delete [] out_buff;
            delete zstrm_p;
        }

        virtual std::streambuf::int_type overflow(std::streambuf::int_type c = traits_type::eof()) {
            zstrm_p->next_in = reinterpret_cast< decltype(zstrm_p->next_in) >(pbase());
            zstrm_p->avail_in = pptr() - pbase();
            while (zstrm_p->avail_in > 0)
            {
                int r = deflate_loop(Z_NO_FLUSH);
                if (r != 0)
                {
                    setp(nullptr, nullptr);
                    return traits_type::eof();
                }
            }
            setp(in_buff, in_buff + buff_size);
            return traits_type::eq_int_type(c, traits_type::eof()) ? traits_type::eof() : sputc(c);
        }

        virtual int sync() {
            // first, call overflow to clear in_buff
            overflow();
            if (! pptr()) return -1;
            // then, call deflate asking to finish the zlib stream
            zstrm_p->next_in = nullptr;
            zstrm_p->avail_in = 0;
            if (deflate_loop(Z_FINISH) != 0) return -1;
            deflateReset(zstrm_p);
            return 0;
        }

        static const std::size_t default_buff_size = (std::size_t)1 << 20;

    private:

        std::streambuf * sbuf_p;
        char * in_buff;
        char * out_buff;
        StreamWrapper * zstrm_p;
        std::size_t buff_size;

    }; // class OStreamBuf

}


#endif // LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__OSTREAMBUF_HPP
