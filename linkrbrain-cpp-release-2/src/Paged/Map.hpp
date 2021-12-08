#ifndef LINKRBRAIN2019__SRC__MAPPED__MAP_HPP
#define LINKRBRAIN2019__SRC__MAPPED__MAP_HPP


#include <sys/mman.h>


namespace Paged {

    struct Map {
        void* _mapped_pointer;
        std::size_t _size;
        int64_t _usage_count;
        int64_t _usage_time;
        std::size_t _locks_count;
        bool _is_dirty;

        inline Map() {
            memset(this, 0, sizeof(*this));
        }

        inline void load(int file_handle, std::size_t start, std::size_t size) {
            _mapped_pointer = mmap(
                NULL,
                size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                file_handle,
                start
            );
            if (_mapped_pointer == MAP_FAILED) {
                throw Exceptions::Exception("Big trouble here: could not map file from " + std::to_string(start) + ". Check the value of /proc/sys/vm/max_map_count (" + std::to_string(start) + ")");
            }
            _size = size;
            _usage_count = 1;
            _usage_time = clock();
            _locks_count = 0;
            _is_dirty = false;
        }
        inline void unload(std::size_t page_size) {
            // // std::cout << "RELEASE " << _mapped_pointer << std::endl;
            // if (msync(_mapped_pointer, page_size, MS_SYNC) == -1) {
            //     throw Exceptions::Exception("error while synchronizing pointer");
            // }
            if (munmap(_mapped_pointer, page_size) == -1) {
                throw Exceptions::Exception("error while unmapping pointer");
            }
        }

        inline const bool is_locked() const {
            return _locks_count;
        }
        inline void lock() {
            _locks_count++;
        }
        inline void unlock() {
            _locks_count--;
        }

        inline const void* read() {
            _usage_count++;
            _usage_time = clock();
            return _mapped_pointer;
        }
        inline void* write() {
            _usage_count++;
            _usage_time = clock();
            _is_dirty = true;
            return _mapped_pointer;
        }
    };

} // Paged


#include <ostream>

// representation
std::ostream& operator << (std::ostream& os, const Paged::Map& map) {
    return os
        << "<Map at "
        << map._mapped_pointer
        << " was used "
        << map._usage_count
        << " times, last "
        << (clock() - map._usage_time) / (double)CLOCKS_PER_SEC
        << " seconds ago"
        << (map.is_locked() ? " (in use)" : "")
        << (map._is_dirty ? " (dirty)" : "")
        << ">";
}

#endif // LINKRBRAIN2019__SRC__MAPPED__MAP_HPP
