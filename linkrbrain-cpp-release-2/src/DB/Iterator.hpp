#ifndef LINKRBRAIN2019LINKRBRAIN2019__SRC__DB__ITERATOR_HPP
#define LINKRBRAIN2019LINKRBRAIN2019__SRC__DB__ITERATOR_HPP


#include "./Cursor.hpp"


namespace DB {

    struct Iterator {
    public:

        Iterator(Cursor* cursor) :
            _cursor(cursor) {}

        inline Iterator& operator++ () {
            _is_readable = _cursor->next();
            return *this;
        }

        inline Cursor& operator * () {
            return * _cursor;
        }

        inline Cursor& operator -> () {
            return * _cursor;
        }

        inline Iterator& begin() {
            const size_t size = _cursor->init();
            if (size == -1) {
                _is_readable = false;
            } else {
                _is_readable = true;
            }
            return *this;
        }

        inline operator bool() const {
            return _is_readable;
        }

        static inline const bool end() {
            return false;
        }

        std::shared_ptr<Cursor>& get_cursor() {
            return _cursor;
        }

    private:

        bool _is_readable;
        std::shared_ptr<Cursor> _cursor;
        size_t _row_index;

    };

} // DB


#endif // LINKRBRAIN2019LINKRBRAIN2019__SRC__DB__ITERATOR_HPP
