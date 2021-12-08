#ifndef LINKRBRAIN2019__SRC__TYPES__VARINTEGER_HPP
#define LINKRBRAIN2019__SRC__TYPES__VARINTEGER_HPP


#include <stdint.h>

#include "Buffering/Reading/Reader.hpp"
#include "Buffering/Writing/Writer.hpp"
#include "Types/Serializable.hpp"
#include "./VarIntegerConversion.hpp"


namespace Types {

    #pragma pack(push, 1)

    template <bool is_signed=true, typename integer_t=int64_t, int reserved_bytes=static_cast<int>((8./7.)*sizeof(integer_t) + (((8./7.)*sizeof(integer_t)) != 0))>
    class VarInteger : public Types::Serializable {
    public:

        inline VarInteger(const integer_t source) {
            set_integer(source);
        }
        inline VarInteger(Buffering::Reading::Reader& reader) {
            _size = 0;
            reader.read(& blocks.unsigned_blocks[_size], 1);
            do {
                if (blocks.unsigned_blocks[_size].is_last) {
                    return;
                }
            } while (reader.read(& blocks.unsigned_blocks[++_size], 1));
        }

        inline VarInteger& operator=(const integer_t source) {
            set_integer(source);
            return *this;
        }

        // get / set

        inline integer_t get_integer() const {
            return VarIntegerConversion::parse(_blocks, _size, is_signed);
        }
        inline void set_integer(integer_t source) {

            if (source == 0) {
                signed_block = {
                    .is_last = true,
                    .is_negative = false,
                    .number = 0,
                };
                _size = 1;
                return;
            }

            if (is_signed) {
                if (source < 0) {
                    signed_block.is_negative = true;
                    source = -source;
                } else {
                    signed_block.is_negative = false;
                }
                signed_block.number = 63 & source;
                source >>= 6;
                signed_block.is_last = false;
            }

            _size = is_signed ? 1 : 0;
            while (source) {
                unsigned_blocks[_size++] = {
                    .is_last = false,
                    .number = 127 & source,
                };
                source >>= 7;
            }
            unsigned_blocks[_size - 1].is_last = true;
        }

        // about size

        inline const int get_size() const {
            return _size;
        }

        // serialization

        void serialize(Buffering::Writing::Writer& writer) const {
            switch (writer.get_format()) {
                // case Buffering::Format::XML:
                case Buffering::Format::Text:
                case Buffering::Format::JSON:
                    writer << get_integer();
                    break;
                case Buffering::Format::Binary:
                    writer.write((const char*) &unsigned_blocks, _size);
                    break;
            }
        }

    private:

        uint8_t _size;
        VarIntegerConversion::blocks_t<reserved_bytes> blocks;

    };

    typedef VarInteger<true, int64_t> VarInt64;
    typedef VarInteger<false, uint64_t> VarUInt64;

    #pragma pack(pop)


} // Types


#endif // LINKRBRAIN2019__SRC__TYPES__VARINTEGER_HPP
