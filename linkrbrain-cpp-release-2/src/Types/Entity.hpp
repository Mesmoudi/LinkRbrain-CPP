#ifndef LINKRBRAIN2019__SRC__TYPES__ENTITY_HPP
#define LINKRBRAIN2019__SRC__TYPES__ENTITY_HPP


#include "./Variant.hpp"
#include "Conversion/Binary.hpp"

#include <string>
#include <atomic>


namespace Types {

    class Entity {
    public:

        Entity() :
            _id(++_counter) {}
        Entity(const std::string& label) :
            _label(label),
            _id(++_counter) {}
        Entity(const size_t id, const std::string& label) :
            _label(label),
            _id(id) {}

        std::string& get_label() {
            return _label;
        }
        const std::string& get_label() const {
            return _label;
        }
        void set_label(const std::string& label) {
            _label = label;
        }

        Types::Variant& get_metadata() {
            return _metadata;
        }
        const Types::Variant& get_metadata() const {
            return _metadata;
        }
        const Types::Variant& get_metadata(const std::string& key) const {
            return _metadata[key];
        }
        void set_metadata(const std::string& key, const Types::Variant& value) {
            _metadata[key] = value;
        }
        void set_metadata(const Types::Variant& value) {
            switch (value.get_type()) {
                case Types::Variant::Undefined:
                case Types::Variant::Null:
                case Types::Variant::Map:
                    _metadata = value;
                    break;
                default:
                    except("wrong Variant type for Entity metadata");
            }
        }

        size_t& get_id() {
            return _id;
        }
        const size_t& get_id() const {
            return _id;
        }
        const size_t get_counter() const {
            return _counter;
        }

    protected:

        size_t _id;
        std::string _label;
        Types::Variant _metadata;

    private:

        static std::atomic<size_t> _counter;

    };

    std::atomic<size_t> Entity::_counter = 0;

} // Types


namespace Conversion::Binary {

    void parse(std::istream& buffer, Types::Entity& destination) {
        parse(buffer, destination.get_id());
        parse(buffer, destination.get_label());
        parse(buffer, destination.get_metadata());
    }

    void serialize(std::ostream& buffer, const Types::Entity& source) {
        serialize(buffer, source.get_id());
        serialize(buffer, source.get_label());
        serialize(buffer, source.get_metadata());
    }

} // Conversion::Binary


#endif // LINKRBRAIN2019__SRC__TYPES__ENTITY_HPP
