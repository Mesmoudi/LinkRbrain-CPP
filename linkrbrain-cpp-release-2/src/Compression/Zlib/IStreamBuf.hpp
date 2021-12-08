#ifndef LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__ISTREAMBUF_HPP
#define LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__ISTREAMBUF_HPP


#include <cassert>

#include "./StreamWrapper.hpp"


namespace Compression::Zlib {

    class IStreamBuf : public std::streambuf {
    public:
        IStreamBuf(std::streambuf * _sbuf_p,
                   std::size_t _buff_size = default_buff_size, bool _auto_detect = true)
            : sbuf_p(_sbuf_p),
              zstrm_p(nullptr),
              buff_size(_buff_size),
              auto_detect(_auto_detect),
              auto_detect_run(false),
              is_text(false)
        {
            assert(sbuf_p);
            in_buff = new char [buff_size];
            in_buff_start = in_buff;
            in_buff_end = in_buff;
            out_buff = new char [buff_size];
            setg(out_buff, out_buff, out_buff);
        }

        IStreamBuf(const IStreamBuf &) = delete;
        IStreamBuf(IStreamBuf &&) = default;
        IStreamBuf & operator = (const IStreamBuf &) = delete;
        IStreamBuf & operator = (IStreamBuf &&) = default;

        virtual ~IStreamBuf()
        {
            delete [] in_buff;
            delete [] out_buff;
            if (zstrm_p) delete zstrm_p;
        }

        virtual std::streambuf::int_type underflow()
        {
            if (this->gptr() == this->egptr())
            {
                // pointers for free region in output buffer
                char * out_buff_free_start = out_buff;
                do
                {
                    // read more input if none available
                    if (in_buff_start == in_buff_end)
                    {
                        // empty input buffer: refill from the start
                        in_buff_start = in_buff;
                        std::streamsize sz = sbuf_p->sgetn(in_buff, buff_size);
                        in_buff_end = in_buff + sz;
                        if (in_buff_end == in_buff_start) break; // end of input
                    }
                    // auto detect if the stream contains text or deflate data
                    if (auto_detect && ! auto_detect_run)
                    {
                        auto_detect_run = true;
                        unsigned char b0 = *reinterpret_cast< unsigned char * >(in_buff_start);
                        unsigned char b1 = *reinterpret_cast< unsigned char * >(in_buff_start + 1);
                        // Ref:
                        // http://en.wikipedia.org/wiki/Gzip
                        // http://stackoverflow.com/questions/9050260/what-does-a-zlib-header-look-like
                        is_text = ! (in_buff_start + 2 <= in_buff_end
                                     && ((b0 == 0x1F && b1 == 0x8B)         // gzip header
                                         || (b0 == 0x78 && (b1 == 0x01      // zlib header
                                                            || b1 == 0x9C
                                                            || b1 == 0xDA))));
                    }
                    if (is_text)
                    {
                        // simply swap in_buff and out_buff, and adjust pointers
                        assert(in_buff_start == in_buff);
                        std::swap(in_buff, out_buff);
                        out_buff_free_start = in_buff_end;
                        in_buff_start = in_buff;
                        in_buff_end = in_buff;
                    }
                    else
                    {
                        // run inflate() on input
                        if (! zstrm_p) zstrm_p = new StreamWrapper(true);
                        zstrm_p->next_in = reinterpret_cast< decltype(zstrm_p->next_in) >(in_buff_start);
                        zstrm_p->avail_in = in_buff_end - in_buff_start;
                        zstrm_p->next_out = reinterpret_cast< decltype(zstrm_p->next_out) >(out_buff_free_start);
                        zstrm_p->avail_out = (out_buff + buff_size) - out_buff_free_start;
                        int ret = inflate(zstrm_p, Z_NO_FLUSH);
                        // process return code
                        if (ret != Z_OK && ret != Z_STREAM_END) throw Exception(zstrm_p, ret);
                        // update in&out pointers following inflate()
                        in_buff_start = reinterpret_cast< decltype(in_buff_start) >(zstrm_p->next_in);
                        in_buff_end = in_buff_start + zstrm_p->avail_in;
                        out_buff_free_start = reinterpret_cast< decltype(out_buff_free_start) >(zstrm_p->next_out);
                        assert(out_buff_free_start + zstrm_p->avail_out == out_buff + buff_size);
                        // if stream ended, deallocate inflator
                        if (ret == Z_STREAM_END)
                        {
                            delete zstrm_p;
                            zstrm_p = nullptr;
                        }
                    }
                } while (out_buff_free_start == out_buff);
                // 2 exit conditions:
                // - end of input: there might or might not be output available
                // - out_buff_free_start != out_buff: output available
                this->setg(out_buff, out_buff, out_buff_free_start);
            }
            return this->gptr() == this->egptr()
                ? traits_type::eof()
                : traits_type::to_int_type(*this->gptr());
        }
    private:
        std::streambuf * sbuf_p;
        char * in_buff;
        char * in_buff_start;
        char * in_buff_end;
        char * out_buff;
        StreamWrapper * zstrm_p;
        std::size_t buff_size;
        bool auto_detect;
        bool auto_detect_run;
        bool is_text;

        static const std::size_t default_buff_size = (std::size_t)1 << 20;
    }; // class IStreamBuf

}


#endif // LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__ISTREAMBUF_HPP
