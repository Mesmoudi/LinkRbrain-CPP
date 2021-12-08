#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__SCORERCACHE_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__SCORERCACHE_HPP


#include "LinkRbrain/Models/Group.hpp"

#include "Logging/Loggable.hpp"
#include "Conversion/Binary.hpp"


namespace LinkRbrain::Scoring::Caching {


    using namespace LinkRbrain::Models;

    enum Status {
        Undefined = -1,
        Empty = 0,
        Projecting = 1,
        Ready = 2,
    };

    template <typename T>
    class ScorerCache : public Logging::Loggable {
    public:

        template <typename T2>
        ScorerCache(const std::vector<Group<T2>>& groups) {
            _groups_count = groups.size();
            _groups_hash = 0;
            _zero.resize(_groups_count);
        }
        virtual ~ScorerCache() {}

        void set_status(const Status& status, const size_t& progress=-1) {
            _status = status;
            _progress = progress;
        }
        const Status& get_status() const {
            return _status;
        }
        const size_t& get_progress() const {
            return _progress;
        }

        virtual void clear() = 0;

        virtual void integrate(const size_t& group_index, const uint32_t& point_hash, const T& value) = 0;
        virtual void integrate(const uint32_t& point_hash, const std::vector<T>& values, const bool replace=true) = 0;
        void integrate(ScorerCache<T>& source, const bool replace=true) {
            source.integrate_into(*this, replace);
        }
        virtual void integrate_into(ScorerCache<T>& destination, const bool replace=true) = 0;

        virtual const std::vector<T> get_score_map(const uint32_t& point_hash) = 0;

        inline const bool is_nonzero(const std::vector<T>& values) const {
            return memcmp(&(values[0]), &(_zero[0]), _groups_count * sizeof(T));
        }

        virtual const std::string get_type_name() const {
            return "ScorerCache";
        }

    protected:

        size_t _groups_count;
        std::vector<T> _zero;

        virtual const std::string get_logger_name() {
            return get_type_name();
        }

        friend class Manager;
        ScorerCache() : _status(Undefined) {}

    private:

        size_t _groups_hash;
        Status _status;
        size_t _progress;

    };


} // LinkRbrain::Scoring::Caching


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__CACHING__SCORERCACHE_HPP
