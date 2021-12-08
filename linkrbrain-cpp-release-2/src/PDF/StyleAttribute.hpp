#ifndef LINKRBRAIN2019__SRC__PDF__STYLEATTRIBUTE_HPP
#define LINKRBRAIN2019__SRC__PDF__STYLEATTRIBUTE_HPP


#include "./None.hpp"


namespace PDF {

    template <typename T>
    class StyleAttribute {
    public:

        enum Status {
            None = -1,
            Unset = 0,
            Set = 1,
        };

        StyleAttribute() :
            _status(Unset)
        {
            if (std::is_arithmetic_v<T>) {
                _value = static_cast<T>(0);
            }
        }
        StyleAttribute(const NoneClass& none) :
            _status(None)
        {
            if (std::is_arithmetic_v<T>) {
                _value = static_cast<T>(0);
            }
        }
        StyleAttribute(const T& value) :
            _status(Set),
            _value(value) {}
        template <typename ...Types>
        StyleAttribute(const Types& ...args) :
            _status(Set),
            _value(args...) {}

        StyleAttribute<T>& operator = (const T& value) {
            _status = Set;
            _value = value;
            return *this;
        }

        operator const T& () const {
            return _value;
        }
        operator T& () {
            return _value;
        }

        const T& get_value() const {
            return _value;
        }
        const bool is_set() const {
            return _status == Set;
        }
        void unset() {
            _status = Unset;
        }
        const StyleAttribute operator | (const StyleAttribute& other) const {
            switch (other._status) {
                case None:
                    return {};
                case Unset:
                    return *this;
                case Set:
                    return other;
            }
        }
        StyleAttribute& operator |= (const StyleAttribute& other) {
            switch (other._status) {
                case None:
                    _status = None;
                    return *this;
                case Unset:
                    return *this;
                case Set:
                    _status = Set;
                    _value = other._value;
                    return *this;
            }
        }

    private:
        Status _status;
        T _value;
    };


} // PDF


#endif // LINKRBRAIN2019__SRC__PDF__STYLEATTRIBUTE_HPP
