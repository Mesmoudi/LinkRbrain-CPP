#ifndef LINKRBRAIN2019__SRC__INDEXING__ITERATION__BASEITERATOR_HPP
#define LINKRBRAIN2019__SRC__INDEXING__ITERATION__BASEITERATOR_HPP


#include <type_traits>


namespace Indexing::Iteration {

    template<typename value_t>
    struct BaseIterator {
        typedef value_t type;
        inline BaseIterator& begin() {
            return *this;
        }
        inline static const bool end() {
            return false;
        }
    };

    template<class DerivedIterator>
    struct is_iterator {
        template<template<typename> class F>
        struct conversion_tester {
            template <typename T>
            conversion_tester(const F<T>&);
        };
        static const bool value = std::is_convertible<DerivedIterator, conversion_tester<BaseIterator>>::value;
    };

} // Indexing::Iteration

#endif // LINKRBRAIN2019__SRC__INDEXING__ITERATION__BASEITERATOR_HPP
