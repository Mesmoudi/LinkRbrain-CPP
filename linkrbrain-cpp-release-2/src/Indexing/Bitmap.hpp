#ifndef LINKRBRAIN2019__SRC__INDEXING__BITMAP_HPP
#define LINKRBRAIN2019__SRC__INDEXING__BITMAP_HPP


#include "Paged/Directory.hpp"
#include "./Index.hpp"


namespace Indexing {

    #pragma pack(push, 1)

    struct BitmapHeader : Paged::Header {
        size_t next_index;
        virtual void set(size_t page_size, const std::string& type) {
            next_index = 0;
        }
    };

    struct BitmapPage {
        char values[0];
    };


    template <typename key_t>
    struct Bitmap : public Index<key_t, bool> {

        Paged::Directory& _directory;
        Paged::File<BitmapHeader, BitmapPage>& _file;

        inline Bitmap(Paged::Directory& directory, const std::string path, const size_t page_size=0)
        : _directory(directory)
        , _file(directory.file<BitmapHeader, BitmapPage>(path, "BITMAP", page_size))
        {
            static_assert(std::is_convertible<key_t, size_t>::value,
                "The key type for a Bitmap index should be convertible to `size_t`."
            );
        }

        void insert(const key_t& key, const bool& value) {
            const size_t index = key;
            const size_t bit_index = index & 7;
            const size_t char_index = index >> 3;
            const size_t value_index = char_index & _file._page_size_mask;
            const size_t page_index = char_index >> _file._page_size_highestbit;
            auto page = _file.page(page_index);
            if (value) {
                page->values[value_index] |= (1 << bit_index);
            } else {
                page->values[value_index] &= ~(1 << bit_index);
            }
        }
        inline size_t append(const bool value) {
            auto header = _file.header();
            size_t index = header->next_index++;
            if (value) {
                insert(index, true);
            }
            return index;
        }

        inline const bool get(const key_t& key) {
            const size_t index = key;
            const size_t bit_index = index & 7;
            const size_t char_index = index >> 3;
            const size_t value_index = char_index & _file._page_size_mask;
            const size_t page_index = char_index >> _file._page_size_highestbit;
            auto page = _file.page(page_index);
            return page->values[value_index] & (1 << bit_index);
        }

        inline const size_t size() {
            auto header = _file.header();
            return header->next_index;
        }

    };

    #pragma pack(pop)

} // Indexing

#endif // LINKRBRAIN2019__SRC__INDEXING__BITMAP_HPP
