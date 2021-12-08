#ifndef LINKRBRAIN2019__SRC__TYPES__NULLABLEPOINTER_HPP
#define LINKRBRAIN2019__SRC__TYPES__NULLABLEPOINTER_HPP


#include "Exceptions/Exception.hpp"


namespace Types {

    template <typename T>
    class NullablePointer {
    public:

        NullablePointer() : _pointer(NULL) {}
        NullablePointer(T* pointer) : _pointer(pointer) {}

        T* operator -> () {
            if (!is_set()) {
                throw Exceptions::Exception("Pointer is not set");
            }
            return _pointer;
        }
        T& operator * () {
            if (!is_set()) {
                throw Exceptions::Exception("Pointer is not set");
            }
            return *_pointer;
        }

        T* get_pointer() {
            return _pointer;
        }
        const T* get_pointer() const {
            return _pointer;
        }

        void set(T* pointer) {
            _pointer = pointer;
        }
        const bool is_set() const {
            return _pointer != NULL;
        }

    private:


        T* _pointer;

    };

} // PDF


#endif // LINKRBRAIN2019__SRC__TYPES__NULLABLEPOINTER_HPP
