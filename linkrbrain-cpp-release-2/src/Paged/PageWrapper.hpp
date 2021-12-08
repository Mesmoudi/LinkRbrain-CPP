#ifndef LINKRBRAIN2019__SRC__MAPPED__PAGEWRAPPER_HPP
#define LINKRBRAIN2019__SRC__MAPPED__PAGEWRAPPER_HPP


#include "./Map.hpp"


namespace Paged {

    template<typename page_t>
    struct PageWrapper {
        Map& _map;
        PageWrapper(Map& map) : _map(map) {
            _map.lock();
        }
        ~PageWrapper() {
            _map.unlock();
        }
        inline page_t& operator*() {
            return * (page_t*) _map.write();
        }
        inline page_t* operator->() {
            return (page_t*) _map.write();
        }
        inline const page_t& operator*() const {
            return * (page_t*) _map.read();
        }
        inline const page_t* operator->() const {
            return (page_t*) _map.read();
        }

    };

} // Paged

#endif // LINKRBRAIN2019__SRC__MAPPED__PAGEWRAPPER_HPP
