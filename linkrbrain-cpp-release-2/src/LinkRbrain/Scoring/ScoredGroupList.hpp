#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__SCOREDGROUPLIST_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__SCOREDGROUPLIST_HPP


#include "./ScoredGroup.hpp"

#include <map>


namespace LinkRbrain::Scoring {

    using namespace LinkRbrain::Models;

    template <typename T>
    class ScoredGroupList : public std::vector<ScoredGroup<T>> {
    public:

        ScoredGroupList(const std::vector<Group<T>>& groups, const size_t count) : _count(count) {
            for (const auto& group : groups) {
                this->push_back({group, count});
            }
        }

        void increment_scores(const size_t query_group_index, const std::vector<T>& values, const T& weight) {
            if (values.size() != this->size()) {
                except("Vector sizes do not match in ScoredGroupList::increment_scores");
            }
            for (size_t dataset_group_index = 0; dataset_group_index < values.size(); dataset_group_index++) {
                (*this)[dataset_group_index].scores[query_group_index]
                    += weight * values[dataset_group_index];
            }
        }

        ScoredGroupList<T> sorted(const size_t limit=-1) {
            // first, insert into multimap
            std::multimap<T, ScoredGroup<T>*> sorted;
            for (ScoredGroup<T>& scored_group : *this) {
                if (scored_group.overall_score) {
                    sorted.insert({scored_group.overall_score, &scored_group});
                }
            }
            // now put this into a vector
            size_t n = 0;
            ScoredGroupList<T> sorted_scored_groups(_count);
            for (auto it=sorted.rbegin(); it!=sorted.rend(); ++it) {
                if (++n > limit) {
                    break;
                }
                sorted_scored_groups.push_back(* it->second);
            }
            // the end!
            return sorted_scored_groups;
        }

        void sort(const size_t limit=-1) {
            *this = sorted(limit);
        }

        const size_t get_count() const {
            return _count;
        }

    private:

        ScoredGroupList(const size_t count) : _count(count) {}
        size_t _count;

    };


} // LinkRbrain::Scoring


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__SCOREDGROUPLIST_HPP
