#ifndef LINKRBRAIN2019__SRC__INDEXING__ITERATION__ANDITERATOR_HPP
#define LINKRBRAIN2019__SRC__INDEXING__ITERATION__ANDITERATOR_HPP


#include "./BaseIterator.hpp"

#include <unordered_set>


namespace Indexing::Iteration {

    template<typename value_t, typename Iterator1, typename Iterator2>
    struct AndIterator : BaseIterator<value_t> {
        Iterator1 _iterator1;
        Iterator2 _iterator2;
        std::unordered_set<value_t> _values1;
        inline AndIterator(Iterator1 iterator1, Iterator2 iterator2)
            : _iterator1(iterator1)
            , _iterator2(iterator2)
            {}
        inline void begin() {
            _values1.insert(_iterator1.begin(), _iterator1.end());
            _iterator2.begin();
            while (_iterator2) {
                if (_values1.find(*_iterator2) != _values1.end()) {
                    return;
                }
                ++_iterator2;
            }
        }
        inline operator const bool() const {
            return _iterator2;
        }
        inline void operator++() {
            if (!_iterator2) {
                return;
            }
            ++_iterator2;
            do {
                if (_values1.find(*_iterator2) != _values1.end()) {
                    return;
                }
                ++_iterator2;
            } while (_iterator2);
        }
        inline const value_t& operator*() const {
            return *_iterator2;
        }
    };

    template<
        typename Iterator1, typename Iterator2,
        typename = typename std::enable_if<is_iterator<Iterator1>::value>::type,
        typename = typename std::enable_if<is_iterator<Iterator2>::value>::type,
        typename = typename std::enable_if<std::is_same<typename Iterator1::type, typename Iterator2::type>::type>::type
    >
    inline AndIterator<typename Iterator1::type, Iterator1, Iterator2>
    operator&&(Iterator1 iterator1, Iterator2 iterator2) {
        return AndIterator<typename Iterator1::type, Iterator1, Iterator2>(iterator1, iterator2);
    }

} // Indexing::Iteration

#endif // LINKRBRAIN2019__SRC__INDEXING__ITERATION__ANDITERATOR_HPP
