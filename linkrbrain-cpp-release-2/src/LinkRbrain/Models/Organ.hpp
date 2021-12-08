#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__ORGAN_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__ORGAN_HPP


#include <istream>

#include "Types/Entity.hpp"
#include "Logging/Loggable.hpp"
#include "Conversion/Binary.hpp"

namespace LinkRbrain::Models {
    template <typename T>
    class Organ;
}
namespace Conversion::Binary {
    template <typename T>
    void parse(std::istream& buffer, LinkRbrain::Models::Organ<T>& destination);
    template <typename T>
    void serialize(std::ostream& buffer, const LinkRbrain::Models::Organ<T>& source);
}


namespace LinkRbrain::Models {

    template <typename T>
    class Organ : public Types::Entity, public Logging::Loggable {
    public:

        Organ(const size_t id, const std::string& label) :
            Entity(id, label) {}
        Organ(std::istream& buffer) {
            Conversion::Binary::parse(buffer, *this);
        }

        virtual const std::string get_logger_name() {
            return "Organ[" + std::to_string(get_id()) + "|" + get_label() + "]";
        }
    };

} // LinkRbrain::Models


namespace Conversion::Binary {

    template <typename T>
    void parse(std::istream& buffer, LinkRbrain::Models::Organ<T>& destination) {
        parse(buffer, (Types::Entity&) destination);
    }

    template <typename T>
    void serialize(std::ostream& buffer, const LinkRbrain::Models::Organ<T>& source) {
        serialize(buffer, (const Types::Entity&) source);
    }

} // Conversion::Binary


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__MODELS__ORGAN_HPP
