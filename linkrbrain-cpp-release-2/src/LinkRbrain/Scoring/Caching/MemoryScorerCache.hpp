#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__MEMORYSCORERCACHE_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__MEMORYSCORERCACHE_HPP


#include "./ScorerCache.hpp"

#include <unordered_map>


namespace LinkRbrain::Scoring::Caching {


    template <typename T>
    class MemoryScorerCache : public ScorerCache<T> {
    public:

        using ScorerCache<T>::ScorerCache;

        virtual void clear() {
            _cache.clear();
            this->set_status(Status::Empty);
        }

        virtual void integrate(const size_t& group_index, const uint32_t& point_hash, const T& value) {
            if (value == static_cast<T>(0.0)) {
                return;
            }
            std::vector<T>& point_scores = _cache[point_hash];
            point_scores.resize(this->_groups_count);
            point_scores[group_index] += value;
        }
        virtual void integrate(const uint32_t& point_hash, const std::vector<T>& values, const bool replace=true) {
            if (values.size() != this->_groups_count) {
                except("Groups count does not match (got", values.size(), "but expected", this->_groups_count, "instead)");
            }
            if (!this->is_nonzero(values)) {
                return;
            }
            std::vector<T>& row = _cache[point_hash];
            if (replace) {
                row = values;
            } else {
                if (row.size() == 0) {
                    row.resize(this->_groups_count, 0);
                }
                for (size_t i=0; i<this->_groups_count; ++i) {
                    row[i] += values[i];
                }
            }
        }
        virtual void integrate_into(ScorerCache<T>& destination, const bool replace=true) {
            std::set<uint32_t> ordered_point_hashes;
            for (const auto& [point_hash, values] : _cache) {
                ordered_point_hashes.insert(point_hash);
            }
            for (const auto& point_hash : ordered_point_hashes) {
                destination.integrate(point_hash, _cache.find(point_hash)->second, replace);
            }
        }

        virtual const std::vector<T> get_score_map(const uint32_t& point_hash) {
            static const std::vector<T> empty;
            const auto it = _cache.find(point_hash);
            if (it != _cache.end()) {
                return it->second;
            }
            return this->_zero;
        }

        virtual const std::string get_type_name() const {
            return "MemoryScorerCache";
        }

    private:

        std::unordered_map<uint32_t, std::vector<T>> _cache;

    };


} // LinkRbrain::Scoring::Caching


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__MEMORYSCORERCACHE_HPP
