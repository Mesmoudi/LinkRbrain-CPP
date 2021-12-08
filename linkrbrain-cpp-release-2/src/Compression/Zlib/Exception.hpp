#ifndef LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__EXCEPTION_HPP
#define LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__EXCEPTION_HPP


#include <exception>


namespace Compression::Zlib {

    class Exception : public std::exception {
    public:
        Exception(z_stream* zstrm_p, int ret) {
            _msg = "Error in Compression::Zlib, ";
            switch (ret) {
                case Z_STREAM_ERROR:
                    _msg += "Z_STREAM_ERROR: ";
                    break;
                case Z_DATA_ERROR:
                    _msg += "Z_DATA_ERROR: ";
                    break;
                case Z_MEM_ERROR:
                    _msg += "Z_MEM_ERROR: ";
                    break;
                case Z_VERSION_ERROR:
                    _msg += "Z_VERSION_ERROR: ";
                    break;
                case Z_BUF_ERROR:
                    _msg += "Z_BUF_ERROR: ";
                    break;
                default:
                    _msg += "[" + std::to_string(ret) + "]: ";
                    break;
            }
            _msg += zstrm_p->msg;
        }
        Exception(const std::string msg) : _msg(msg) {}
        const char * what() const noexcept { return _msg.c_str(); }

    private:

        std::string _msg;
    }; // class Exception

}


#endif // LINKRBRAIN2019__SRC__COMPRESSION__ZLIB__EXCEPTION_HPP
