#ifndef LINKRBRAIN2019__SRC__INDEXING__FIXEDPRIMARY_HPP
#define LINKRBRAIN2019__SRC__INDEXING__FIXEDPRIMARY_HPP


#include "Paged/Header.hpp"
#include "Paged/Directory.hpp"
#include "./Index.hpp"


namespace Indexing {

    #pragma pack(push, 1)

    template<typename value_t>
    struct FixedPrimaryHeader : Paged::Header {
        size_t next_index;
        size_t data_size;
        inline void set(const size_t page_size, const std::string& file_type) {
            Paged::Header::set(page_size, file_type);
            next_index = 0;
            data_size = sizeof(value_t);
        }
        inline void check(const size_t page_size, const std::string& file_type, const std::string& file_path) {
            Paged::Header::check(page_size, file_type, file_path);
            if (data_size != sizeof(value_t)) {
                throw Paged::BadHeaderException(file_path, "data size", sizeof(value_t), data_size);
            }
        }
    };

    template<typename value_t>
    struct FixedPrimaryPage {
        value_t values[0];
    };

    #pragma pack(pop)


    template<typename key_t, typename value_t>
    struct FixedPrimary : Index<key_t, value_t> {

        Paged::Directory& _directory;
        Paged::File<FixedPrimaryHeader<value_t>, FixedPrimaryPage<value_t>>& _file;
        size_t _values_per_page;

        inline FixedPrimary(Paged::Directory& directory, const std::string path, const size_t page_size=0)
        : _directory(directory)
        , _file(directory.file<FixedPrimaryHeader<value_t>, FixedPrimaryPage<value_t>>(path, "FIXEDPRIMARY", page_size))
        , _values_per_page(_file._page_size / sizeof(value_t))
        {
            //
        }
        ~FixedPrimary() {
        }

        virtual void insert(const key_t& key, const value_t& value) {
            const size_t index = key;
            size_t page_index = index / _values_per_page;
            size_t onpage_index = index - (page_index * _values_per_page);
            auto page = _file.page(page_index);
            memcpy(page->values + onpage_index, &value, sizeof(value_t));
        }
        inline size_t insert(const value_t& value) {
            auto header = _file.header();
            size_t index = header->next_index++;
            insert(index, value);
            return index;
        }

        virtual const value_t get(const key_t& key) {
            // std::cout << ":-/" << std::endl;
            const size_t index = key;
            auto page = _file.page(index / _values_per_page);
            // std::cout << ":-) [" << page._map._mapped_pointer << "] " << index % _values_per_page << std::endl;
            value_t value = page->values[index % _values_per_page];
            // std::cout << value << std::endl;
            return value;
            // size_t page_index = ;
            // std::cout << ":-(" << std::endl;
            // size_t onpage_index = index - (page_index * _values_per_page);
            // std::cout << ":-D" << std::endl;
            // return _file.page(page_index)->values[onpage_index];
        }
        inline value_t& get_reference(const size_t& index) {
            // std::cout << ":-/" << std::endl;
            auto page = _file.page(index / _values_per_page);
            // std::cout << ":-) [" << page._map._mapped_pointer << "] " << index % _values_per_page << std::endl;
            return page->values[index % _values_per_page];
        }

        inline const size_t size() const {
            auto header = _file.header();
            return header->next_index;
        }

        inline void clear() {
            _file.clear();
        }

    };

} // Indexing

#endif // LINKRBRAIN2019__SRC__INDEXING__FIXEDPRIMARY_HPP
