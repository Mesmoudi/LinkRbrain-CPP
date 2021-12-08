#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__SCOREDGROUP_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__SCOREDGROUP_HPP


#include "../Models/Group.hpp"

#include <vector>


namespace LinkRbrain::Scoring {

    using namespace LinkRbrain::Models;

    template <typename T>
    static const Group<T> empty_group("(empty group for correlation)");

    template <typename T>
    struct ScoredGroup {

        ScoredGroup(const Group<T>& _group, size_t _count) :
            group(_group),
            overall_score(NAN)
        {
            scores.resize(_count, static_cast<T>(0));
        }

        const Group<T>& group;
        T overall_score;
        std::vector<T> scores;
    };


} // LinkRbrain::Scoring


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SCORING__SCOREDGROUP_HPP
