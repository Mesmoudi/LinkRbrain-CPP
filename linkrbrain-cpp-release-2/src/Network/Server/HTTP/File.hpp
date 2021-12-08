#ifndef LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__FILE_HPP
#define LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__FILE_HPP


#include "Types/MiniMap.hpp"

#include <string>
#include <sstream>


namespace Network::Server::HTTP {


    struct File {
    public:

        File(const char* _key, const char* _filename, const char* _content_type, const char* _transfer_encoding) :
            key(_key ? _key : ""),
            filename(_filename ? _filename : ""),
            content_type(_content_type ? _content_type : ""),
            transfer_encoding(_transfer_encoding ? _transfer_encoding : "") {}

        std::string key;
        std::string filename;
        std::string content_type;
        std::string transfer_encoding;
        std::string data;

    };


} // Network::Server::HTTP


#endif // LINKRBRAIN2019__SRC__NETWORK__SERVER__HTTP__FILE_HPP
