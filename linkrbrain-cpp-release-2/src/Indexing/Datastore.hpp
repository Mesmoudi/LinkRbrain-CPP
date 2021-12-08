#ifndef LINKRBRAIN2019__SRC__INDEXING__DATASTORE_HPP
#define LINKRBRAIN2019__SRC__INDEXING__DATASTORE_HPP


#include "Paged/Directory.hpp"
#include "./Index.hpp"


namespace Indexing {


    #pragma pack(push, 1)

    template<typename value_t>
    struct DatastoreHeader : Paged::Header {
        std::size_t next_offset;
        bool is_reserving;
        inline void set(const std::size_t page_size, const std::string& file_type) {
            Paged::Header::set(page_size, file_type);
            next_offset = 0;
            is_reserving = false;
        }
        inline void check(const std::size_t page_size, const std::string& file_type, const std::string& file_path) {
            Paged::Header::check(page_size, file_type, file_path);
        }
    };

    #pragma pack(pop)


    template <typename value_t, typename size_t>
    struct Datastore {

        Paged::File<DatastoreHeader<value_t>, char[]>& _file;
        const std::size_t _page_size;

        inline Datastore(Paged::Directory& directory, const std::string path, const std::size_t page_size=0)
        : _file(directory.file<DatastoreHeader<value_t>, char[]>(path, "DATASTORE", page_size))
        , _page_size(_file._page_size)
        {}

        template <typename item_t>
        inline const std::size_t realsizeof(const std::vector<item_t> vector) {
            std::size_t size = sizeof(size_t);
            for (auto& item : vector) {
                size += realsizeof(item);
            }
            return size;
        }
        inline const std::size_t realsizeof(const std::string& string) {
            return sizeof(size_t) + string.size();
        }
        template <typename item_t>
        inline const std::size_t realsizeof(const item_t& item) {
            return sizeof(item);
        }

        inline const std::size_t reserve(const std::size_t size) {
            while (_file.header()->is_reserving);
            _file.header()->is_reserving = true;
            const std::size_t offset = _file.header()->next_offset;
            _file.header()->next_offset += size;
            _file.header()->is_reserving = false;
            return offset;
        }

        struct cursor_t {
            Datastore& _datastore;
            std::size_t _page_size;
            std::size_t _page_index;
            std::size_t _onpage_offset;
            std::size_t _onpage_size;
            char* _onpage_data;
            inline cursor_t(Datastore& datastore, const std::size_t offset)
                : _datastore(datastore)
                , _page_size(datastore._file._page_size)
                , _page_index(offset / _page_size)
                , _onpage_offset(offset % _page_size)
                , _onpage_size(_page_size - _onpage_offset)
                , _onpage_data(*_datastore._file.page(_page_index) + _onpage_offset) {}

            inline void write(const void* data, std::size_t size) {
                if (size <= _onpage_size) {
                    memcpy(_onpage_data, data, size);
                    _onpage_data += size;
                    _onpage_size -= size;
                    return;
                }
                memcpy(_onpage_data, data, _onpage_size);
                data = (const char*)data + _onpage_size;
                size -= _onpage_size;
                _onpage_data = *_datastore._file.page(++_page_index);
                _onpage_size = _page_size;
                write(data, size);
            }
            inline void write(const std::string& string) {
                const size_t size = string.size();
                write(size);
                write(string.data(), size);
            }
            template <typename item_t>
            inline void write(const std::vector<item_t>& vector) {
                const size_t size = vector.size();
                write(size);
                for (auto& item : vector) {
                    write(item);
                }
            }
            template <typename item_t>
            inline void write(const item_t& item) {
                write(&item, sizeof(item_t));
            }

            inline void read(void* data, std::size_t size) {
                if (size <= _onpage_size) {
                    memcpy(data, _onpage_data, size);
                    _onpage_data += size;
                    _onpage_size -= size;
                    return;
                }
                memcpy(data, _onpage_data, _onpage_size);
                data = (char*)data + _onpage_size;
                size -= _onpage_size;
                _onpage_data = *_datastore._file.page(++_page_index);
                _onpage_size = _page_size;
                read(data, size);
            }
            inline void read(std::string& string) {
                size_t size;
                read(size);
                string.resize(size);
                read(&string[0], size);
            }
            template <typename item_t>
            inline void read(std::vector<item_t>& vector) {
                size_t size;
                read(size);
                for (size_t i=0; i<size; ++i) {
                    item_t item;
                    read(item);
                    vector.push_back(item);
                }
            }
            template <typename item_t>
            inline void read(item_t& item) {
                read(&item, sizeof(item_t));
            }
        };

        inline const std::size_t insert(const value_t& value) {
            const std::size_t size = realsizeof(value);
            const std::size_t offset = reserve(size);
            cursor_t(*this, offset).write(value);
            return offset;
        }
        inline const value_t get(const std::size_t offset) {
            value_t value;
            cursor_t(*this, offset).read(value);
            return value;
        }

    };

} // Indexing

#endif // LINKRBRAIN2019__SRC__INDEXING__DATASTORE_HPP
