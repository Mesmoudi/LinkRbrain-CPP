#ifndef LINKRBRAIN2019__SRC__TYPES__SERIALIZABLE_HPP
#define LINKRBRAIN2019__SRC__TYPES__SERIALIZABLE_HPP


namespace Buffering {
    namespace Writing {
        class Writer;
    } // Writing
    namespace Reading {
        class Reader;
    } // Writing
} // Buffering


namespace Types {

    class Serializable {
    public:

        virtual void serialize(Buffering::Writing::Writer& writer) const = 0;
        virtual void deserialize(Buffering::Reading::Reader& reader) = 0;

    };

} // Types


#include "Buffering/Writing/OStreamWriter.hpp"

#include <ostream>

inline std::ostream& operator << (std::ostream& os, const Types::Serializable& value) {
    Buffering::Writing::OStreamWriter writer(os, Buffering::Format::JSON);
    value.serialize(writer);
    return os;
}


#endif // LINKRBRAIN2019__SRC__TYPES__SERIALIZABLE_HPP
