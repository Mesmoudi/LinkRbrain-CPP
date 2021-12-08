#ifndef LINKRBRAIN2019__SRC__TYPES__INTEGERS_HPP
#define LINKRBRAIN2019__SRC__TYPES__INTEGERS_HPP


#pragma pack(push)
#pragma pack(1)

#include <stdint.h>
#include <type_traits>
#include <string>


namespace Types {

    template<typename base_t, std::size_t bytes_length>
    struct WeirdInt {
        base_t value : 8 * bytes_length;

        // conversion from another integer

        inline WeirdInt() {}

        template<typename other_t, typename = typename std::enable_if<std::is_convertible<other_t, base_t>::value>::type>
        inline WeirdInt(const other_t source) : value((base_t) source) {}

        template<typename other_base_t, std::size_t other_bytes_length>
        inline WeirdInt(const WeirdInt<other_base_t, other_bytes_length> source) : value((base_t) source.value) {}

        // conversion to other stuff

        template<typename other_t, typename = typename std::enable_if<std::is_convertible<base_t, other_t>::value>::type>
        inline operator const other_t() const {return (other_t) value;}

        template<typename other_base_t, std::size_t other_bytes_length>
        inline operator const WeirdInt<other_base_t, other_bytes_length>() const {return WeirdInt<other_base_t, other_bytes_length>(value);}

        inline operator const std::string() const {return std::string(value);}

        // comparison with a regular integer

        #define WEIRDINT_DECLARE_COMPARISON_OPERATOR_INT(OP) \
            template<typename other_t, typename = typename std::enable_if<std::is_convertible<other_t, base_t>::value>::type> \
            inline const bool operator OP (const other_t other) const {return value OP other;}
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_INT(==);
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_INT(!=);
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_INT(>=);
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_INT(<=);
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_INT(>);
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_INT(<);

        // comparison with another weird integer

        #define WEIRDINT_DECLARE_COMPARISON_OPERATOR_WEIRDINT(OP) \
            template<typename other_base_t, std::size_t other_bytes_length> \
            inline const bool operator OP (const WeirdInt<other_base_t, other_bytes_length> other) const {return value OP other.value;}
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_WEIRDINT(==);
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_WEIRDINT(!=);
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_WEIRDINT(>=);
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_WEIRDINT(<=);
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_WEIRDINT(>);
        WEIRDINT_DECLARE_COMPARISON_OPERATOR_WEIRDINT(<);

        // self-incrementation

        inline void operator++ () {++value;}
        inline void operator-- () {--value;}

        // incrementation

        template<typename other_t, typename = typename std::enable_if<std::is_convertible<other_t, base_t>::value>::type>
        inline WeirdInt<base_t, bytes_length>& operator+= (const other_t other) {value += other; return *this;}

        // other operations with regular integers

        template<typename other_t, typename = typename std::enable_if<std::is_convertible<other_t, base_t>::value>::type>
        inline const base_t operator<< (const other_t& other) const {return value << other;}

        template<typename other_t, typename = typename std::enable_if<std::is_convertible<other_t, base_t>::value>::type>
        inline const base_t operator>> (const other_t& other) const {return value >> other;}

        template<typename other_t, typename = typename std::enable_if<std::is_convertible<other_t, base_t>::value>::type>
        inline const base_t operator+ (const other_t& other) const {return value + other;}

        template<typename other_t, typename = typename std::enable_if<std::is_convertible<other_t, base_t>::value>::type>
        inline const base_t operator- (const other_t& other) const {return value - other;}

        template<typename other_t, typename = typename std::enable_if<std::is_convertible<other_t, base_t>::value>::type>
        inline const base_t operator* (const other_t& other) const {return value * other;}

        template<typename other_t, typename = typename std::enable_if<std::is_convertible<other_t, base_t>::value>::type>
        inline const base_t operator/ (const other_t& other) const {return value / other;}

        template<typename other_t, typename = typename std::enable_if<std::is_convertible<other_t, base_t>::value>::type>
        inline const base_t operator% (const other_t& other) const {return value % other;}

    };


    template<typename base_t, std::size_t bytes_length>
    std::ostream& operator << (std::ostream& os, const WeirdInt<base_t, bytes_length>& weirdint) {
        return (os << weirdint.value);
    }


    // incrementation

    template<typename base_t, std::size_t bytes_length, typename other_t, typename = typename std::enable_if<std::is_convertible<base_t, other_t>::value>::type>
    other_t& operator += (other_t& other, const WeirdInt<base_t, bytes_length>& weirdint) {
        return (other += weirdint.value);
    }

    // operations

    template<typename base_t, std::size_t bytes_length, typename other_t, typename = typename std::enable_if<std::is_convertible<base_t, other_t>::value>::type>
    other_t operator * (const other_t& other, const WeirdInt<base_t, bytes_length>& weirdint) {
        return (other * weirdint.value);
    }

    template<typename base_t, std::size_t bytes_length, typename other_t, typename = typename std::enable_if<std::is_convertible<base_t, other_t>::value>::type>
    other_t operator + (const other_t& other, const WeirdInt<base_t, bytes_length>& weirdint) {
        return (other + weirdint.value);
    }

    // comparison

    template<typename base_t, std::size_t bytes_length, typename other_t, typename = typename std::enable_if<std::is_convertible<base_t, other_t>::value>::type>
    const bool operator != (const other_t other, const WeirdInt<base_t, bytes_length>& weirdint) {
        return (other != weirdint.value);
    }

    template<typename base_t, std::size_t bytes_length, typename other_t, typename = typename std::enable_if<std::is_convertible<base_t, other_t>::value>::type>
    const bool operator < (const other_t other, const WeirdInt<base_t, bytes_length>& weirdint) {
        return (other < weirdint.value);
    }

    template<typename base_t, std::size_t bytes_length, typename other_t, typename = typename std::enable_if<std::is_convertible<base_t, other_t>::value>::type>
    const bool operator > (const other_t other, const WeirdInt<base_t, bytes_length>& weirdint) {
        return (other > weirdint.value);
    }


    // names definition

    typedef WeirdInt<uint32_t, 3> uint24_t;
    typedef WeirdInt<uint64_t, 5> uint40_t;
    typedef WeirdInt<uint64_t, 6> uint48_t;
    typedef WeirdInt<uint64_t, 7> uint56_t;


    struct uint0_t {

        inline uint0_t() {}

        template<typename other_t>
        inline uint0_t(const other_t source) {}

        template<typename other_base_t, std::size_t other_bytes_length>
        inline uint0_t(const WeirdInt<other_base_t, other_bytes_length> source) {}

        template<typename other_t>
        inline uint0_t& operator= (const other_t other) {return *this;}
    };


    #pragma pack(pop)

}



namespace std {

    template<typename base_t, size_t bytes_length>
    struct hash<Types::WeirdInt<base_t, bytes_length>> {
        size_t operator()(const Types::WeirdInt<base_t, bytes_length>& integer) const {
            return integer.value;
        }
    };

};

#endif // LINKRBRAIN2019__SRC__TYPES__INTEGERS_HPP
