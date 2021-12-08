#ifndef LINKRBRAIN2019__SRC__TYPES__VARINTEGERBLOCK_HPP
#define LINKRBRAIN2019__SRC__TYPES__VARINTEGERBLOCK_HPP


#include <stdint.h>


namespace Types {

    #pragma pack(push, 1)

    union VarIntegerBlock {
        char data;
        struct {
            bool is_last : 1;
            bool is_negative : 1;
            uint64_t value6 : 6;
        };
        struct {
            bool is_last_ : 1;
            uint64_t value7 : 7;
        };
    };

    #pragma pack(pop)

} // Types


#endif // LINKRBRAIN2019__SRC__TYPES__VARINTEGERBLOCK_HPP
