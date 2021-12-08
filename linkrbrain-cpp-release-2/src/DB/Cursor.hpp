#ifndef LINKRBRAIN2019LINKRBRAIN2019__SRC__DB__CURSOR_HPP
#define LINKRBRAIN2019LINKRBRAIN2019__SRC__DB__CURSOR_HPP


#include "Types/Variant.hpp"

#include <vector>


namespace DB {

    class Cursor {
    public:

        virtual ~Cursor() {}
        virtual const size_t init() = 0;
        virtual const bool next() = 0;

        virtual const bool get_isnull(const size_t column_index) = 0;
        virtual const std::vector<std::string>& get_text() = 0;
        virtual const std::string get_text(const size_t column_index) = 0;
        virtual const std::string get_text(const std::string& column_name) = 0;

        inline const size_t get_row_size() const {
            return _row_size;
        }

    protected:

        size_t _row_size;

    };

} // DB


#endif // LINKRBRAIN2019LINKRBRAIN2019__SRC__DB__CURSOR_HPP
