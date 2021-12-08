#ifndef LINKRBRAIN2019__SRC__INDEXING__EXCEPTIONS_HPP
#define LINKRBRAIN2019__SRC__INDEXING__EXCEPTIONS_HPP


#include "Exceptions/Exception.hpp"
#include "Paged/File.hpp"


namespace Indexing {

    struct KeyNotFoundException : public Exceptions::Exception {
        template<typename file_header_t, typename file_page_t, typename key_t>
        inline KeyNotFoundException(Paged::File<file_header_t, file_page_t>& file, key_t key)
        : Exception("Key not found in `" + file._file_path + "`: `" + ::std::to_string(key) + "`")
        {}
        template<typename file_header_t, typename file_page_t, typename key_t>
        inline KeyNotFoundException(Paged::File<file_header_t, file_page_t>& file, key_t key, const ::std::string& details)
        : Exception("Key not found in `" + file._file_path + "`: `" + ::std::to_string(key) + "` (" + details + ")")
        {}
        template<typename key_t>
        inline KeyNotFoundException(key_t key)
        : Exception("Key not found: `" + ::std::to_string(key) + "`")
        {}
    };

} // Indexing


#endif // LINKRBRAIN2019__SRC__INDEXING__EXCEPTIONS_HPP
