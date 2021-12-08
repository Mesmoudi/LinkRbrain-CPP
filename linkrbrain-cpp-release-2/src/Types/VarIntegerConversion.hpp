#ifndef LINKRBRAIN2019__SRC__TYPES__VARINTEGERCONVERSION_HPP
#define LINKRBRAIN2019__SRC__TYPES__VARINTEGERCONVERSION_HPP


#include <stdint.h>


namespace Types {

    struct VarIntegerConversion {

        struct signed_block_t {
            bool is_last : 1;
            bool is_negative : 1;
            uint64_t number : 6;
        };
        struct block_t {
            bool is_last : 1;
            uint64_t number : 7;
        };

        template <size_t N>
        union blocks_t {
            signed_block_t signed_block;
            block_t unsigned_blocks[N];
        };

        static inline int64_t parse(const blocks_t<10>& blocks, const bool is_signed=true) {
            int position = 0;
            // read the first byte
            int64_t result = is_signed ? blocks.signed_block.number : blocks.unsigned_blocks[0].number;
            int64_t shift = is_signed ? 6 : 7;
            const bool is_negative = is_signed && blocks.signed_block.is_negative;
            // read the rest
            while (!blocks.unsigned_blocks[position++].is_last) {
                result |= (int64_t) blocks.unsigned_blocks[position].number << shift;
                shift += 7;
            }
            return is_negative ? -result : result;
        }
        static inline int64_t parse(Buffering::Reading::Reader& reader, const bool is_signed=true) {
            blocks_t<10> blocks;
            int position = 0;
            // read the first byte
            reader.read(& blocks.unsigned_blocks[position], 1);
            int64_t result = is_signed ? blocks.signed_block.number : blocks.unsigned_blocks[0].number;
            int64_t shift = is_signed ? 6 : 7;
            const bool is_negative = is_signed && blocks.signed_block.is_negative;
            // read the rest
            while (!blocks.unsigned_blocks[position++].is_last && reader.read(& blocks.unsigned_blocks[position], 1)) {
                result |= (int64_t) blocks.unsigned_blocks[position].number << shift;
                shift += 7;
            }
            return is_negative ? -result : result;
        }

        static inline void convert(int64_t source, uint8_t& size, blocks_t<10>& blocks, const bool is_signed) {

            if (source == 0) {
                size = 1;
                blocks.signed_block = {
                    .is_last = true,
                    .is_negative = false,
                    .number = 0,
                };
                return;
            }

            if (is_signed) {
                if (source < 0) {
                    blocks.signed_block.is_negative = true;
                    source = -source;
                } else {
                    blocks.signed_block.is_negative = false;
                }
                blocks.signed_block.number = 63 & source;
                source >>= 6;
                blocks.signed_block.is_last = false;
            }

            size = is_signed ? 1 : 0;
            while (source) {
                blocks.unsigned_blocks[size++] = {
                    .is_last = false,
                    .number = 127 & source,
                };
                source >>= 7;
            }
            blocks.unsigned_blocks[size - 1].is_last = true;
        }

    };

} // Types


#endif // LINKRBRAIN2019__SRC__TYPES__VARINTEGERCONVERSION_HPP
