#ifndef LINKRBRAIN2019__SRC__INDEXING__ITERATION__ORITERATOR_HPP
#define LINKRBRAIN2019__SRC__INDEXING__ITERATION__ORITERATOR_HPP


#include "./BaseIterator.hpp"

#include <unordered_set>


namespace Indexing::Iteration {

    template<typename value_t, typename Iterator1, typename Iterator2>
    struct OrIterator : BaseIterator<value_t> {
        Iterator1 _iterator1;
        Iterator2 _iterator2;
        std::unordered_set<value_t> _values1;
        typename std::unordered_set<value_t>::iterator _it1;
        bool _has_finished_values1;
        inline OrIterator(Iterator1 iterator1, Iterator2 iterator2)
            : _iterator1(iterator1)
            , _iterator2(iterator2)
            , _it1(_values1.end())
            {}
        inline void begin() {
            _values1.insert(_iterator1.begin(), _iterator1.end());
            _it1 = _values1.begin();
            _has_finished_values1 = (_it1 == _values1.end());
            //
            _iterator2.begin();
            if (_has_finished_values1) {
                while (_iterator2) {
                    if (_values1.find(*_iterator2) == _values1.end()) {
                        return;
                    }
                    ++_iterator2;
                }
            }
        }
        inline operator const bool() const {
            return !_has_finished_values1 || (bool)_iterator2;
        }
        inline void operator++() {
            if (_has_finished_values1) {
                while (_iterator2) {
                    ++_iterator2;
                    if (_values1.find(*_iterator2) == _values1.end()) {
                        return;
                    }
                }
            } else {
                ++_it1;
                if (_it1 == _values1.end()) {
                    _has_finished_values1 = true;
                }
            }
        }
        inline const value_t& operator*() const {
            return _has_finished_values1 ? *_iterator2 : *_it1;
        }
    };

    template<
        typename Iterator1, typename Iterator2,
        typename = typename std::enable_if<is_iterator<Iterator1>::value>::type,
        typename = typename std::enable_if<is_iterator<Iterator2>::value>::type,
        typename = typename std::enable_if<std::is_same<typename Iterator1::type, typename Iterator2::type>::type>::type
    >
    inline OrIterator<typename Iterator1::type, Iterator1, Iterator2>
    operator||(Iterator1 iterator1, Iterator2 iterator2) {
        return OrIterator<typename Iterator1::type, Iterator1, Iterator2>(iterator1, iterator2);
    }

} // Indexing::Iteration

#endif // LINKRBRAIN2019__SRC__INDEXING__ITERATION__ORITERATOR_HPP
