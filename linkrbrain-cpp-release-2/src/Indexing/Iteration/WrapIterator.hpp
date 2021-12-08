#ifndef LINKRBRAIN2019__SRC__INDEXING__ITERATION__WRAPITERATOR_HPP
#define LINKRBRAIN2019__SRC__INDEXING__ITERATION__WRAPITERATOR_HPP


#include "./BaseIterator.hpp"

#include <vector>


namespace Indexing::Iteration {

    template <typename container_t>
    struct WrapIterator : BaseIterator<typename container_t::value_type> {
        container_t _container;
        typename container_t::iterator _container_iterator;
        inline WrapIterator(container_t container) : _container(container) {}
        inline WrapIterator& begin() {
            _container_iterator = _container.begin();
            return *this;
        }
        inline operator const bool() const {
            return _container_iterator != _container.end();
        }
        inline void operator++() {
            ++_container_iterator;
        }
        inline const typename container_t::value_type& operator*() const {
            return *_container_iterator;
        }
    };

    // template<
    //     typename Iterator,
    //     typename value_t,
    //     typename = typename std::enable_if<is_iterator<Iterator>::type>::type,
    //     typename = typename std::enable_if<std::is_same<typename Iterator::type, value_t>::type>::type
    // >
    // inline operator WrapIterator<std::vector<value_t>>(Iterator& iterator) {
    //     WrapIterator<std::vector<value_t>> result_iterator;
    //     return result_iterator;
    // }

} // Indexing::Iteration

#endif // LINKRBRAIN2019__SRC__INDEXING__ITERATION__WRAPITERATOR_HPP
