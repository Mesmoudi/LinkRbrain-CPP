#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__PARSER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__PARSER_HPP


#include <string>

#include "LinkRbrain/Models/Dataset.hpp"


namespace LinkRbrain::Parsing {

    class Parser {
    public:

        template <typename T>
        static void parse(const std::string& path, LinkRbrain::Models::Dataset<T>& dataset) {
        }

    };


} // LinkRbrain::Parsing


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__PARSING__PARSER_HPP
