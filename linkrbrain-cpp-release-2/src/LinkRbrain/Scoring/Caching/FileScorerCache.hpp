#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__FILESCORERCACHE_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__FILESCORERCACHE_HPP


#include "./ScorerCache.hpp"

#include <vector>
#include <string>
#include <filesystem>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>


namespace LinkRbrain::Scoring::Caching {


    template <typename T>
    class FileScorerCache : public ScorerCache<T> {
    public:

        template <typename T2>
        FileScorerCache(std::vector<Group<T2>> groups, const std::filesystem::path& path) :
            ScorerCache<T>(groups),
            _path(path)
        {
            _f = fopen(_path.c_str(), "r+b");
            if (_f == NULL) {
                this->get_logger().debug("Could not open file " + _path.native() + ", " + strerror(errno));
                _f = fopen(_path.c_str(), "w+b");
                if (_f == NULL) {
                    except("Could not create file " + _path.native() + ", " + strerror(errno));
                } else {
                    this->get_logger().debug("Created file " + _path.native() + "");
                }
            } else {
                this->get_logger().debug("Opened", _path.native());
            }
        }

        ~FileScorerCache() {
            std::string message = fclose(_f) ? strerror(errno) : "";
            this->get_logger().debug("Closed", _path.native(), message);
        }

        virtual void clear() {
            this->set_status(Empty);
            ftruncate(fileno(_f), 0);
        }

        inline const size_t compute_offset(const size_t& group_index, const uint32_t& point_hash) const {
            return 4096 + sizeof(T) * (point_hash * this->_groups_count + group_index);
        }

        virtual void integrate(const size_t& group_index, const uint32_t& point_hash, const T& value) {
            if (value == static_cast<T>(0.0)) {
                return;
            }
            const size_t offset = compute_offset(group_index, point_hash);
            // load value
            T final_value;
            fseek(_f, offset, SEEK_SET);
            fread(&final_value, sizeof(T), 1, _f);
            // increment and save value
            final_value += value;
            fseek(_f, offset, SEEK_SET);
            fwrite(&final_value, sizeof(T), 1, _f);
        }
        virtual void integrate(const uint32_t& point_hash, const std::vector<T>& values, const bool replace=true) {
            if (!this->is_nonzero(values)) {
                return;
            }
            const size_t offset = compute_offset(0, point_hash);
            if (replace) {
                fseek(_f, offset, SEEK_SET);
                fwrite(&(values[0]), sizeof(T), this->_groups_count, _f);
            } else {
                // load value
                std::vector<T> final_values;
                final_values.resize(this->_groups_count);
                fseek(_f, offset, SEEK_SET);
                fread(&(final_values[0]), sizeof(T), this->_groups_count, _f);
                // increment and save value
                for (size_t i=0; i<this->_groups_count; ++i) {
                    final_values[i] += values[i];
                }
                fseek(_f, offset, SEEK_SET);
                fwrite((&final_values[0]), sizeof(T), this->_groups_count, _f);
            }
        }
        virtual void integrate_into(ScorerCache<T>& destination, const bool replace=true) {
            // prepare data recipients
            size_t point_hash = 0;
            std::vector<T> values(this->_groups_count, 0.0);
            // prepare offset boundaries & step
            size_t offset_min = compute_offset(0, 0);
            size_t offset_step = compute_offset(0, 1) - offset_min;
            fseek(_f, 0, SEEK_END);
            size_t offset_max = ftell(_f);
            // browse entire file
            fseek(_f, offset_min, SEEK_SET);
            for (size_t offset=offset_min; offset<offset_max; offset+=offset_step) {
                ++point_hash;
                fread(&(values[0]), sizeof(T), this->_groups_count, _f);
                if (this->is_nonzero(values)) {
                    destination.integrate(point_hash, values);
                }
            }
        }

        virtual const std::vector<T> get_score_map(const uint32_t& point_hash) {
            const size_t offset = compute_offset(0, point_hash);
            std::vector<T> result;
            result.resize(this->_groups_count);
            fseek(_f, offset, SEEK_SET);
            fread(&(result[0]), sizeof(T), this->_groups_count, _f);
            return result;
        }

    protected:

        virtual const std::string get_type_name() const {
            return "FileScorerCache";
        }

        friend class Manager;
        FileScorerCache() : ScorerCache<T>() {}

    private:

        const std::filesystem::path _path;
        FILE* _f;

    };


} // LinkRbrain::Scoring::Caching


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__FILESCORERCACHE_HPP
