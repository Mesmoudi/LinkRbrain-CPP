#ifndef LINKRBRAIN2019__SRC__MAPPED__FILE_HPP
#define LINKRBRAIN2019__SRC__MAPPED__FILE_HPP


#include "./Map.hpp"
#include "./PageWrapper.hpp"
#include "./Exceptions.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


namespace Paged {

    template<typename header_t, typename page_t>
    struct File {

        std::string _file_path;
        int _file_handle;
        const std::string _file_type;

        const std::size_t _page_size;
        std::size_t _page_size_highestbit;
        std::size_t _page_size_mask;

        std::size_t _pageblock_size;
        std::size_t _file_size;
        std::size_t& _total_cache_size;
        std::unordered_map<std::size_t, Map> _cache;
        Map _header;

        inline File(std::size_t& total_cache_size, const std::string file_path, const std::string file_type, const std::size_t page_size, const std::size_t pageblock_size)
        : _file_path(file_path)
        , _file_handle(open(_file_path.c_str(), O_RDWR | O_CREAT, 0666))
        , _file_type(file_type)
        , _page_size(page_size)
        , _page_size_highestbit(0)
        , _pageblock_size(pageblock_size)
        , _total_cache_size(total_cache_size) {
            // ensure everything goes okay with the file
            if (_file_handle == -1) {
                throw FileSystemException(file_path, "open");
            }
            // highest bit of page size
            for (std::size_t i=0, n=8*sizeof(std::size_t); i<n; i++) {
                if (_page_size == (1 << i)) {
                    _page_size_highestbit = i;
                    break;
                }
            }
            if (_page_size_highestbit == 0) {
                throw Exceptions::Exception("Page size should be a power of two, but was set to " + std::to_string(_page_size));
            }
            _page_size_mask = _page_size - 1;
            // file size
            struct stat stat_buffer;
            fstat(_file_handle, &stat_buffer);
            _file_size = stat_buffer.st_size;
            // header check/set
            if (_file_size == 0) {
                resize(_page_size);
                _header.load(_file_handle, 0, _page_size);
                header()->set(_page_size, _file_type);
            } else {
                _header.load(_file_handle, 0, _page_size);
                header()->check(_page_size, _file_type, _file_path);
            }
        }
        inline ~File() {
        }

        inline void clear() {
            resize(0);
            resize(_page_size);
            _header.load(_file_handle, 0, _page_size);
            header()->set(_page_size, _file_type);
        }
        inline void resize(const std::size_t size) {
            if (ftruncate(_file_handle, size) != 0) {
                throw FileSystemException(_file_path, "resize");
            }
            _file_size = size;
        }
        inline Map& get_map(const std::size_t page_index) {
            std::size_t start = page_index * _page_size;
            if (start >= _file_size) {
                resize(start + _pageblock_size);
            }
            //
            auto it = _cache.find(page_index);
            if (it != _cache.end()) {
                return it->second;
            }
            Map& map = _cache[page_index];
            map.load(_file_handle, start, _page_size);
            _total_cache_size += _page_size;
            return map;
        }

        inline const bool remove_map(const std::size_t page_index, const bool force_removal=false) {
            auto it = _cache.find(page_index);
            if (it != _cache.end()) {
                Map& map = it->second;
                if (force_removal || !map.is_locked()) {
                    map.unload(_page_size);
                    _cache.erase(it);
                    _total_cache_size -= _page_size;
                    return true;
                }
            }
            return false;
        }

        inline header_t* header() {
            return (header_t*) _header._mapped_pointer;
        }
        inline PageWrapper<page_t> page(const std::size_t page_index) {
            return PageWrapper<page_t>(get_map(page_index + 1));
        }

    };

} // Paged


#endif // LINKRBRAIN2019__SRC__MAPPED__FILE_HPP
