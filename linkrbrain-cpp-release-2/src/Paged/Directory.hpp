#ifndef LINKRBRAIN2019__SRC__MAPPED__DIRECTORY_HPP
#define LINKRBRAIN2019__SRC__MAPPED__DIRECTORY_HPP


#include "./Header.hpp"
#include "./File.hpp"

#include "Logging/Loggable.hpp"


#include <thread>
#include <atomic>
#include <unordered_map>

#include <unistd.h>
#include <sys/stat.h>


namespace Paged {

    struct Manager : public Logging::Loggable {

        size_t _default_page_size;
        size_t _default_pageblock_size;

        std::atomic<bool> _is_open;

        size_t _max_cache_size;
        size_t _cache_size;
        std::atomic<bool> _is_cleaning_cache;
        size_t _cache_cleaning_count;
        std::thread* _cache_cleaning_thread;

        std::unordered_map<std::string, File<Header, char>*> _files_by_name;
        std::unordered_map<int, File<Header, char>*> _files_by_handle;

        inline Manager(size_t default_page_size=4096, size_t default_pageblock_size=1048576, size_t max_cache_size=1073741824L)
        : _default_page_size(default_page_size)
        , _default_pageblock_size(default_pageblock_size)
        , _is_open(true)
        , _max_cache_size(max_cache_size)
        , _cache_size(0)
        , _is_cleaning_cache(false)
        , _cache_cleaning_count(0) {
            // start the cleaning thread
            _cache_cleaning_thread = new std::thread(clean_cache_daemon, this);
            _cache_cleaning_thread->detach();
        }
        inline ~Manager() {
            _is_open = false;
            if (_cache_cleaning_thread->joinable()) {
                _cache_cleaning_thread->join();
            }
            get_logger().debug("Destruction: terminating filesystem manager cache cleaning daemon (called", _cache_cleaning_count, "times)");
            // terminate cache
            clean_cache(*this, 0.0);
            get_logger().notice("Destruction: cleaned cache");
            // terminate open files
            for (auto it=_files_by_handle.begin(); it!=_files_by_handle.end(); it++) {
                fsync(it->first);
                close(it->first);
                get_logger().debug("Destruction: closed file: `" + it->second->_file_path + "`");
            }
            get_logger().notice("Destruction: closed all files");
        }

        static void clean_cache(Manager& manager, double rate, bool force_removal=true) {
            if (manager._is_cleaning_cache) {
                return;
            }
            manager._is_cleaning_cache = true;
            manager._cache_cleaning_count++;
            manager.get_logger().debug("Cache cleaning: getting started; filled:", manager._cache_size, "/", manager._max_cache_size);
            // sorted mapped memory
            std::multimap<int64_t, std::pair<int, size_t>> sorted;
            int64_t now = clock();
            for (auto it=manager._files_by_handle.begin(); it!=manager._files_by_handle.end(); it++) {
                int file_handle = it->first;
                for (auto it2=it->second->_cache.begin(); it2!=it->second->_cache.end(); it2++) {
                    size_t page_offset = it2->first;
                    auto& map = it2->second;
                    if (map.is_locked()) {
                        continue;
                    }
                    // std::cout << now << " - " << map._usage_time << " = " << (now - map._usage_time) << std::endl;
                    sorted.insert(std::pair<int64_t, std::pair<int, size_t>>(
                        // map._usage_count * (map._usage_time - now),
                        (now - map._usage_time) / (map._usage_count + 1),
                        // (now - map._usage_time),
                        std::pair<int, size_t>(
                            file_handle,
                            page_offset
                        )
                    ));
                }
            }
            // clean least used
            manager.get_logger().debug("Cache cleaning: finished sorting");
            size_t initial = sorted.size();
            size_t left = rate * initial;
            size_t reached = 0;
            for (auto it=sorted.rbegin(); it!=sorted.rend(); it++) {
                // if (left == 0) {
                //     std::cout << it->first << std::endl;
                //     continue;
                // } else {
                //     std::cout << " * " << it->first << std::endl;
                // }
                auto& file = * manager._files_by_handle[it->second.first];
                if (file.remove_map(it->second.second, force_removal)) {
                    reached++;
                    // --left;
                    if (--left == 0) {
                        break;
                    }
                }
            }
            // go back to sleep
            manager._is_cleaning_cache = false;
            manager.get_logger().debug("Cache cleaning: done; initial:", initial, " / reached:", reached);
        }
        static void clean_cache_daemon(Manager* _manager) {
            Manager& manager = * _manager;
            // get_logger().debug("Start cache cleaning daemon.");
            while (manager._is_open) {
                // check every millisecond
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                while (manager._cache_size < manager._max_cache_size || manager._is_cleaning_cache) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                // clean if conditions are met
                clean_cache(manager, 0.25);
            }
        }

        template<typename header_t, typename page_t>
        inline File<header_t, page_t>&
        file(std::string file_path, std::string file_type, size_t page_size=0, size_t pageblock_size=0) {
            file_path += "." + file_type;
            auto it = _files_by_name.find(file_path);
            if (it != _files_by_name.end()) {
                return (File<header_t, page_t>&) it->second;
            }
            if (page_size == 0) {
                page_size = _default_page_size;
            }
            if (pageblock_size == 0) {
                pageblock_size = _default_pageblock_size;
            }
            File<Header, char>* file = new File<Header, char>(_cache_size, file_path, file_type, page_size, pageblock_size);
            _files_by_name.insert(std::pair<std::string, File<Header, char>*>(
                file_path,
                file
            ));
            _files_by_handle.insert(std::pair<int, File<Header, char>*>(
                file->_file_handle, file
            ));
            return (File<header_t, page_t>&) *file;
        }


        struct Directory {
            Manager& _directory;
            std::string _directory_path;
            inline Directory(Manager& directory, const std::string& subdirectory_path)
            : _directory(directory)
            , _directory_path(subdirectory_path) {
                // append a trailing slash to the path if necessary
                if (*_directory_path.rbegin() != '/') {
                    _directory_path += '/';
                }
                // open or create the given directory
                size_t path_slash_position = 0;
                while (true) {
                    path_slash_position = _directory_path.find('/', path_slash_position + 1);
                    if (path_slash_position == -1) {
                        break;
                    }
                    std::string path = _directory_path.substr(0, path_slash_position);
                    try {
                        if (open(path.c_str(), O_RDONLY) == -1)
                            throw FileSystemException(path, "open");
                    } catch (FileSystemException& e) {
                        if (mkdir(path.c_str(), 0777) != 0)
                            throw FileSystemException(path, "create");
                    }
                }
            }
            inline ~Directory() {
            }
            template<typename header_t, typename page_t>
            inline File<header_t, page_t>&
            file(std::string file_path, std::string file_type, size_t page_size=0, size_t pageblock_size=0) {
                return _directory.file<header_t, page_t>(_directory_path + file_path, file_type, page_size, pageblock_size);
            }
            inline Directory directory(const std::string& subdirectory_path) {
                return _directory.directory(_directory_path + subdirectory_path);
            }
        };


        inline Directory directory(const std::string& subdirectory_path) {
            return Directory(*this, subdirectory_path);
        }

        protected:

            virtual const std::string get_logger_name() {
                return "Paged::Manager";
            }

    };

    typedef Manager::Directory Directory;

} // Paged


#endif // LINKRBRAIN2019__SRC__MAPPED__DIRECTORY_HPP
