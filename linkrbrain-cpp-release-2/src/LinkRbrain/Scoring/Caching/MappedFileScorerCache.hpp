#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__MAPPEDFILESCORERCACHE_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__MAPPEDFILESCORERCACHE_HPP


#include "./ScorerCache.hpp"
#include "Logging/Loggable.hpp"
#include "Indexing/FixedPrimary.hpp"

#include <vector>
#include <string>
#include <filesystem>


namespace LinkRbrain::Scoring::Caching {


    template <typename T>
    class MappedFileScorerCache : public ScorerCache<T> {
    public:

        template <typename T2>
        MappedFileScorerCache(std::vector<Group<T2>> groups, const std::string& path) :
            ScorerCache<T>(groups),
            _path(path),
            _manager(sysconf(_SC_PAGE_SIZE), 1<<20, 1<<28),
            _directory(_manager, _path.parent_path()),
            _index(_directory, _path.filename())
        {}

        virtual void clear() {
            this->set_status(Empty);
            _index.clear();
        }

        virtual void integrate(const size_t& group_index, const uint32_t& point_hash, const T& value) {
            std::cout << "group_index = " << group_index << '\n';
            std::cout << "point_hash = " << point_hash << '\n';
            std::cout << "value = " << value << '\n';
            std::cout << "(point_hash * this->_groups_count + group_index) = " << (point_hash * this->_groups_count + group_index) << '\n';
            _index.get_reference(point_hash * this->_groups_count + group_index) += value;
        }
        virtual void integrate(const uint32_t& point_hash, const std::vector<T>& values, const bool replace=true) {
            except("Not implemented");
        }
        virtual void integrate_into(ScorerCache<T>& destination, const bool replace=true) {
            except("Not implemented");
        }

        virtual const std::vector<T> get_score_map(const uint32_t& point_hash) {
            std::vector<T> result;
            result.resize(this->_groups_count);
            lseek(_index._file._file_handle, _index._file._page_size + sizeof(T)*point_hash*this->_groups_count, SEEK_SET);
            read(_index._file._file_handle, &(result[0]), this->_groups_count*sizeof(T));
            return result;
        }

        virtual const std::string get_type_name() const {
            return "MappedFileScorerCache";
        }

    protected:

        virtual const std::string get_logger_name() {
            return "MappedFileScorerCache[" + _path.native() + "]";
        }

        friend class Manager;
        MappedFileScorerCache() :
            ScorerCache<T>(),
            _manager(sysconf(_SC_PAGE_SIZE), 1<<10, 1<<10),
            _directory(_manager, "/tmp/"),
            _index(_directory, "linkrbrain.fakemappedfile") {}

    private:

        const std::filesystem::path _path;
        Paged::Manager _manager;
        Paged::Directory _directory;
        Indexing::FixedPrimary<size_t, T> _index;

    };


} // LinkRbrain::Scoring::Caching


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__MAPPEDFILESCORERCACHE_HPP
