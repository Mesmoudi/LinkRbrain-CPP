#ifndef LINKRBRAIN2019__SRC__INDEXING__ARRAY_HPP
#define LINKRBRAIN2019__SRC__INDEXING__ARRAY_HPP


#include "Paged/Directory.hpp"
#include "./Iteration/BaseIterator.hpp"
#include "./Index.hpp"


namespace Indexing {

    #pragma pack(push, 1)

    template<typename value_t>
    struct ArrayHeader : public Paged::Header {
        std::size_t value_size;
        std::size_t next_page_index;
        inline void set(const std::size_t page_size, const std::string& file_type) {
            Paged::Header::set(page_size, file_type);
            value_size = sizeof(value_t);
            next_page_index = 0;
        }
        inline void check(const std::size_t page_size, const std::string& file_type, const std::string& file_path) {
            Paged::Header::check(page_size, file_type, file_path);
            if (value_size != sizeof(value_t))
                throw Paged::BadHeaderException(file_path, "value size", sizeof(value_t), value_size);
        }
    };


    template<typename value_t>
    struct ArrayPage {
        std::size_t count;
        std::size_t next;
        value_t values[0];
    };


    #pragma pack(pop)


    template<typename value_t>
    struct Array {

        typedef ArrayHeader<value_t> header_t;
        typedef ArrayPage<value_t> page_t;

        Paged::File<header_t, page_t>& _file;
        std::size_t _values_per_page;


        inline Array(Paged::Directory& directory, const std::string path, const std::size_t page_size=0)
        : _file(directory.file<header_t, page_t>(path, "ARRAY", page_size))
        , _values_per_page((_file._page_size - sizeof(std::size_t) - sizeof(std::size_t)) / sizeof(value_t))
        {}

        inline const std::size_t new_page_index() {
            return _file.header()->next_page_index++;
        }

        inline const std::size_t create_array() {
            return new_page_index();
        }

        void insert(const std::size_t first_page_index, const value_t& value) {
            auto page = _file.page(first_page_index);
            std::size_t onpage_index = page->count % _values_per_page;
            if (page->count != 0 && onpage_index == 0) {
                std::size_t next_page_index = new_page_index();
                auto next_page = _file.page(next_page_index);
                memcpy(&*next_page, &*page, _file._page_size);
                page->next = next_page_index;
            }
            page->count++;
            page->values[onpage_index] = value;
        }

        inline const std::set<value_t> get(const std::size_t first_page_index) {
            std::set<value_t> values;
            std::size_t page_index = first_page_index;
            bool is_first_page = true;
            while (true) {
                auto page = _file.page(page_index);
                value_t* value_begin = page->values + (is_first_page ? (page->count % _values_per_page) : _values_per_page) - 1;
                value_t* value_end = page->values - 1;
                if (value_begin == value_end) {
                    value_begin += _values_per_page;
                }
                for (value_t* value=value_begin; value!=value_end; value--) {
                    values.emplace_hint(values.cbegin(), *value);
                }
                if (page->next == 0) {
                    break;
                }
                page_index = page->next;
                is_first_page = false;
            }
            return values;
        }

        struct Iterator : Iteration::BaseIterator<value_t> {
            Array<value_t>& _array;
            std::size_t _page_index;
            bool _is_first_page;
            std::vector<value_t> _values;
            typename std::vector<value_t>::iterator _values_it;
            //
            inline Iterator(Array<value_t>& array)
                : _array(array)
                {}
            inline Iterator(Array<value_t>& array, const std::size_t first_page_index)
                : _array(array)
                , _page_index(first_page_index)
                {}
            inline void _load_values() {
                auto page = _array._file.page(_page_index);
                value_t* value_begin = page->values + (_is_first_page ? (page->count % _array._values_per_page) : _array._values_per_page) - 1;
                value_t* value_end = page->values - 1;
                if (value_begin == value_end) {
                    value_begin += _array._values_per_page;
                }
                //
                _values.resize(0);
                for (value_t* value=value_begin; value!=value_end; value--) {
                    _values.push_back(*value);
                }
                _values_it = _values.begin();
                //
                _is_first_page = false;
                _page_index = page->next;
            }
            inline void initialize() {
                _is_first_page = true;
                _load_values();
            }
            inline void operator++() {
                ++_values_it;
                if (_values_it == _values.end() && _page_index) {
                    _load_values();
                }
            }
            inline operator const bool() const {
                return (_values_it != _values.end());
            }
            inline const value_t& operator*() const {
                return *_values_it;
            }
        };

        inline const std::size_t count(const std::size_t first_page_index) {
            auto page = _file.page(first_page_index);
            return page->count;
        }
        inline Iterator isin(const std::size_t first_page_index) {
            return Iterator(*this, first_page_index);
        }
    };


    template<typename value_t>
    std::ostream& operator << (std::ostream& os, const Array<value_t>& btree) {
        os << "\n";
        return os;
    }

} // Indexing

#endif // LINKRBRAIN2019__SRC__INDEXING__ARRAY_HPP
