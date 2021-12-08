#ifndef CPPP____INCLUDE____CONVERSION__HELPERS_HPP
#define CPPP____INCLUDE____CONVERSION__HELPERS_HPP


// buffering
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <filesystem>
// containers
#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <array>
#include <list>
#include <deque>
#include <stack>
#include <forward_list>
#include <map>
#include <unordered_map>
// exception
#include "Exceptions/GenericExceptions.hpp"


namespace Conversion {

    #define CPPP____CONVERSION____HELPERS____PARSE \
    template <typename T> \
    void parse(const std::string& string, T& destination) { \
        std::stringstream buffer(string); \
        parse(buffer, destination); \
    } \
    template <typename T> \
    T parse(std::istream& buffer) { \
        T result; \
        parse(buffer, result); \
        return result; \
    } \
    template <typename T> \
    T parse(const std::string& string) { \
        T result; \
        parse(string, result); \
        return result; \
    } \
    template <typename T> \
    void parse_file(const std::filesystem::path& path, T& destination) { \
        std::ifstream buffer(path); \
        parse(buffer, destination); \
    } \
    template <typename T> \
    T parse_file(const std::filesystem::path& path) { \
        T result; \
        parse##_file(path, result); \
        return result; \
    } \
    template <typename T, size_t N> void parse(std::istream& buffer, std::array<T, N>& iterator) { iterator_parse(buffer, iterator); } \
    template <typename T> void parse(std::istream& buffer, std::deque<T>& iterator) { iterator_parse(buffer, iterator); } \
    template <typename T> void parse(std::istream& buffer, std::forward_list<T>& iterator) { iterator_parse(buffer, iterator); } \
    template <typename T> void parse(std::istream& buffer, std::list<T>& iterator) { iterator_parse(buffer, iterator); } \
    template <typename T> void parse(std::istream& buffer, std::set<T>& iterator) { iterator_parse(buffer, iterator); } \
    template <typename T> void parse(std::istream& buffer, std::stack<T>& iterator) { iterator_parse(buffer, iterator); } \
    template <typename T> void parse(std::istream& buffer, std::unordered_set<T>& iterator) { iterator_parse(buffer, iterator); } \
    template <typename T> void parse(std::istream& buffer, std::vector<T>& iterator) { iterator_parse(buffer, iterator); } \
    template <typename K, typename V> void parse(std::istream& buffer, std::unordered_map<K, V>& map) { map_parse(buffer, map); } \
    template <typename K, typename V> void parse(std::istream& buffer, std::map<K, V>& map) { map_parse(buffer, map); }


    #define CPPP____CONVERSION____HELPERS____SERIALIZE \
    template <typename T> \
    const std::string serialize(const T& source) { \
        std::stringstream buffer; \
        serialize(buffer, source); \
        return buffer.str(); \
    } \
    template <typename T> \
    void serialize(std::string& string, const T& source) { \
        std::stringstream buffer(string); \
        serialize(buffer, source); \
    } \
    template <typename T> \
    void serialize_file(const std::filesystem::path& path, const T& source) { \
        std::ofstream buffer(path); \
        if (!buffer.is_open()) { \
            throw Exceptions::NotFoundException("Could not open file as stream: ", path); \
        } \
        serialize(buffer, source); \
    } \
    template <typename T, size_t N> void serialize(std::istream& buffer, const std::array<T, N>& iterator) { iterator_serialize(buffer, iterator); } \
    template <typename T> void serialize(std::istream& buffer, const std::deque<T>& iterator) { iterator_serialize(buffer, iterator); } \
    template <typename T> void serialize(std::istream& buffer, const std::forward_list<T>& iterator) { iterator_serialize(buffer, iterator); } \
    template <typename T> void serialize(std::istream& buffer, const std::list<T>& iterator) { iterator_serialize(buffer, iterator); } \
    template <typename T> void serialize(std::istream& buffer, const std::set<T>& iterator) { iterator_serialize(buffer, iterator); } \
    template <typename T> void serialize(std::istream& buffer, const std::stack<T>& iterator) { iterator_serialize(buffer, iterator); } \
    template <typename T> void serialize(std::istream& buffer, const std::unordered_set<T>& iterator) { iterator_serialize(buffer, iterator); } \
    template <typename T> void serialize(std::istream& buffer, const std::vector<T>& iterator) { iterator_serialize(buffer, iterator); } \
    template <typename K, typename V> void serialize(std::istream& buffer, const std::unordered_map<K, V>& map) { map_serialize(buffer, map); } \
    template <typename K, typename V> void serialize(std::istream& buffer, const std::map<K, V>& map) { map_serialize(buffer, map); }


} // Conversion


#endif // CPPP____INCLUDE____CONVERSION__HELPERS_HPP
