#ifndef CPPP____INCLUDE____TYPES__VARIANT_HPP
#define CPPP____INCLUDE____TYPES__VARIANT_HPP


#include <cmath>
#include <variant>
#include <map>
#include <vector>
#include <string>
#include <ostream>
#include <unordered_map>


namespace Types {

    class Variant;
    struct VariantNull{
        const bool operator== (const VariantNull& other) const { return true; }
    };
    typedef std::vector<Variant> VariantVector;
    typedef std::map<std::string, Variant> VariantMap;

} // Types


#include <Types/DateTime.hpp>


namespace Types {

    typedef std::variant<
        VariantNull,
        bool,
        char,
        int64_t,
        double,
        std::string,
        VariantVector,
        VariantMap,
        Types::DateTime
    > VariantBase;

    class Variant : public VariantBase {
    public:

        struct Exception : public std::bad_variant_access {
            Exception(const std::string& _message = "") : message(_message) {}
            virtual const char* what() const noexcept { return message.c_str(); }
            const std::string message;
        };

        enum Type {
            Null = 0,
            Boolean = 1,
            Character = 2,
            Integer = 3,
            Floating = 4,
            String = 5,
            Vector = 6,
            Map = 7,
            DateTime = 8,
            Undefined = std::variant_npos,
        };

        // Use default std::variant constructor

        using VariantBase::VariantBase;

        // Retrieve information about type

        const Type get_type() const {
            return (const Type) index();
        }
        const std::string& get_type_name(const Type& type) const {
            if (type == std::variant_npos) {
                return _undefined_type_name;
            }
            return _type_names[type];
        }
        const std::string& get_type_name() const {
            return get_type_name(get_type());
        }

        // Value getters

        template <typename T>
        T& get() {
            return std::get<T>(*this);
        }
        template <typename T>
        const T& get() const {
            return std::get<T>(*this);
        }
        template <size_t I>
        std::variant_alternative_t<I, VariantBase>& get() {
            return std::get<I>(*this);
        }
        template <size_t I>
        const std::variant_alternative_t<I, VariantBase>& get() const {
            return std::get<I>(*this);
        }

        // conversion operators

        template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
        operator const T () const {
            return get<int64_t>();
        }
        template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
        operator const T () const {
            return get<double>();
        }
        operator const char () const {
            return get<char>();
        }
        operator const bool () const {
            return get<bool>();
        }
        operator const std::string () const {
            return get<std::string>();
        }
        operator const Types::DateTime () const {
            return get<Types::DateTime>();
        }
        template <typename T>
        operator const std::map<std::string, T> () const {
            std::map<std::string, T> result;
            for (const auto& [key, value] : get<VariantMap>()) {
                result[key] = (T) value;
            }
            return result;
        }
        template <typename T>
        operator const std::vector<T> () const {
            std::vector<T> result;
            for (const T& value : get<VariantVector>()) {
                result.push_back((T) value);
            }
            return result;
        }

        // Accessors

        template<class T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
        Variant& operator[](T index) {
            switch (get_type()) {
                case Vector:
                    {
                        VariantVector& vector = get<VariantVector>();
                        if (std::abs((int64_t)index) >= vector.size()) {
                            throw Exception("Out of range index for vector of size " + std::to_string(vector.size()) + ": " + std::to_string(index));
                        }
                        if (index < 0) {
                            index += vector.size();
                        }
                        return vector[index];
                    }
                default:
                    throw Exception("Illegal use of operator [] with integer operand `" + std::to_string(index) + "`  on Variant of type " + get_type_name());
            }
        }
        template<class T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
        const Variant& operator[](T index) const {
            switch (get_type()) {
                case Vector:
                    {
                        const VariantVector& vector = get<VariantVector>();
                        if (std::abs((int64_t)index) >= vector.size()) {
                            throw Exception("Out of range index for vector of size " + std::to_string(vector.size()) + ": " + std::to_string(index));
                        }
                        if (index < 0) {
                            index = vector.size() + index;
                        }
                        return vector[index];
                    }
                default:
                    throw Exception("Illegal use of operator [] with integer operand `" + std::to_string(index) + "` on Variant of type " + get_type_name());
            }
        }
        template<class T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
        const Variant& get(T index, const Variant& default_value=VariantNull()) const {
            switch (get_type()) {
                case Vector: {
                    {
                        const VariantVector& vector = get<VariantVector>();
                        if (std::abs((int64_t)index) >= vector.size()) {
                            return default_value;
                        }
                        if (index < 0) {
                            index += vector.size();
                        }
                        return vector[index];
                    }
                }
                default:
                    throw Exception("Illegal use of method get() with integer `" + std::to_string(index) + "` on Variant of type " + get_type_name());
            }
        }

        const Variant& get(const std::string& key, const Variant& default_value=VariantNull()) const {
            switch (get_type()) {
                case Map: {
                    {
                        const VariantMap& map = get<VariantMap>();
                        const auto& result = map.find(key);
                        if (result == map.end()) {
                            return default_value;
                        }
                        return result->second;
                    }
                }
                default:
                    throw Exception("Illegal use of method get() with key `" + key + "` on Variant of type " + get_type_name());
            }
        }
        const Variant& operator[](const std::string& key) const {
            switch (get_type()) {
                case Map: {
                    {
                        const VariantMap& map = get<VariantMap>();
                        const auto& result = map.find(key);
                        if (result == map.end()) {
                            throw Exception("Could not find key `" + key + "` in Variant");
                        }
                        return result->second;
                    }
                }
                default:
                    throw Exception("Illegal use of operator [] with string operand `" + key + "` on Variant of type " + get_type_name());
            }
        }
        const Variant& operator[](const char* key) const {
            return operator[](std::string(key));
        }
        Variant& operator[](const std::string& key) {
            switch (get_type()) {
                case Undefined:
                case Null:
                    emplace<VariantMap>();
                case Map:
                    return get<VariantMap>()[key];
                default:
                    throw Exception("Illegal use of operator [] with string operand `" + key + "` on Variant of type " + get_type_name());
            }
        }
        Variant& operator[](const char* key) {
            return operator[](std::string(key));
        }

        template<class T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
        const bool has(const T& index) const {
            switch (get_type()) {
                case Vector:
                    {
                        return (std::abs((int64_t)index) < get_vector().size());
                    }
                default:
                    throw Exception("Illegal use of method has() with integer operand `" + std::to_string(index) + "` on Variant of type " + get_type_name());
            }
        }
        const bool has(const std::string& key) const {
            if (get_type() == Map) {
                const auto& map = get<VariantMap>();
                return (map.find(key) != map.end());
            }
            return false;
        }
        const bool has(const char* key) const {
            return has(std::string(key));
        }

        template <typename K, typename V>
        Variant get(const K& key_or_index, const V& default_value) {
            if (!has(key_or_index)) {
                return default_value;
            }
            return (*this)[key_or_index];
        }
        template <typename K, typename V>
        const Variant& get(const K& key_or_index, const V& default_value) const {
            if (!has(key_or_index)) {
                return default_value;
            }
            return (*this)[key_or_index];
        }

        // comparison

        template<class T, std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>
        const bool operator== (const T& value) const {
            switch (get_type()) {
                case Integer:
                    return get<Integer>() == value;
                case Floating:
                    return get<Floating>() == value;
                default:
                    throw Exception("Cannot compare with arithmetic value: " + get_type_name());
            }
        }

        // special functions

        template <typename T>
        Variant& push_back(const T& value) {
            switch (get_type()) {
                case Null:
                case Undefined:
                    emplace<VariantVector>();
                case Vector:
                    get<VariantVector>().push_back(value);
                    return get<VariantVector>().back();
                default:
                    throw Exception("Cannot push_back to " + get_type_name());
            }
        }

        Variant& push_back() {
            return push_back(VariantNull());
        }

        Variant& push_back(const Variant& value) {
            return push_back<Variant>(value);
        }

        const size_t size() const {
            switch (get_type()) {
                case Map:
                    return get<Map>().size();
                case Vector:
                    return get<Types::Variant::Vector>().size();
                case String:
                    return get<String>().size();
                default:
                    throw Exception("Cannot get size of " + get_type_name());
            }
        }

        const bool contains(const std::string& search, const bool case_insensitive=false, const bool is_first=true) const {
            switch (get_type()) {
                case String:
                    if (case_insensitive) {
                        std::string lowered = get<std::string>();
                        std::transform(lowered.begin(), lowered.end(), lowered.begin(), tolower);
                        return lowered.find(search) != std::string::npos;
                    } else {
                        return get<std::string>().find(search) != std::string::npos;
                    }
                case Vector:
                    for (const Variant& item : get<VariantVector>()) {
                        if (item.contains(search, case_insensitive, false)) {
                            return true;
                        }
                    }
                    break;
                case Map:
                    for (const auto& [key, value] : get<VariantMap>()) {
                        if (value.contains(search, case_insensitive, false)) {
                            return true;
                        }
                    }
                    break;
                default:
                    break;
            }
            return false;
        }

        // Specific conversions

        // Initialize from type
        Variant(const Type& type) {
            switch (type) {
                case Types::Variant::Undefined:
                case Types::Variant::Null:
                    emplace<Types::Variant::Null>();
                    break;
                case Types::Variant::Boolean:
                    emplace<Types::Variant::Boolean>();
                    break;
                case Types::Variant::Character:
                    emplace<Types::Variant::Character>();
                    break;
                case Types::Variant::Integer:
                    emplace<Types::Variant::Integer>();
                    break;
                case Types::Variant::Floating:
                    emplace<Types::Variant::Floating>();
                    break;
                case Types::Variant::String:
                    emplace<Types::Variant::String>();
                    break;
                case Types::Variant::Vector:
                    emplace<Types::Variant::Vector>();
                    break;
                case Types::Variant::Map:
                    emplace<Types::Variant::Map>();
                    break;
                case Types::Variant::DateTime:
                    emplace<Types::Variant::DateTime>();
                    break;
            }
        }

        // Initialize null value
        Variant() {
            emplace<Null>();
        }
        void clear() {
            emplace<Null>();
        }
        // Initialize string value from pointer to character
        Variant(const char* source) {
            emplace<std::string>(source);
        }
        Variant& operator = (const char* source) {
            emplace<std::string>(source);
            return *this;
        }
        // Initialize string value from character array
        template <size_t N>
        Variant(const char (&source)[N]) {
            emplace<std::string>(source, N);
        }
        template <size_t N>
        Variant& operator = (const char (&source)[N]) {
            emplace<std::string>(source, N); return *this;
        }
        // Initialize vector value from initializer list
        template <typename T>
        Variant(const std::initializer_list<T>& source) {
            emplace<VariantVector>(source.begin(), source.end());
        }
        template <typename T>
        Variant& operator = (const std::initializer_list<T>& source) {
            emplace<VariantVector>(source.begin(), source.end());
            return *this;
        }
        // Initialize vector value from array
        template <typename T, size_t N>
        Variant(const T (&items)[N]) {
            emplace<VariantVector>(items, items + N);
        }
        // Initialize vector value from vector
        template <typename T>
        Variant(const std::vector<T>& source) {
            emplace<VariantVector>(source.begin(), source.end());
        }
        template <typename T>
        Variant& operator = (const std::vector<T>& source) {
            emplace<VariantVector>(source.begin(), source.end());
            return *this;
        }
        // Initialize vector value from initializer list of strings
        Variant& operator = (const std::initializer_list<const char*>& source) {
            emplace<VariantVector>();
            for (const char* item : source) {
                get<VariantVector>().push_back(item);
            }
            return *this;
        }
        // Initialize map value
        template <typename Value>
        Variant(const std::map<std::string, Value>& source) {
            emplace<VariantMap>(source.begin(), source.end());
        }
        template <typename Value>
        Variant& operator = (const std::map<std::string, Value>& source) {
            emplace<VariantMap>(source.begin(), source.end());
            return *this;
        }
        template <typename Value>
        Variant(const std::unordered_map<std::string, Value>& source) {
            emplace<VariantMap>(source.begin(), source.end());
        }
        template <typename Value>
        Variant& operator = (const std::unordered_map<std::string, Value>& source) {
            emplace<VariantMap>(source.begin(), source.end());
            return *this;
        }
        Variant(const std::initializer_list<VariantMap::value_type>& source) {
            emplace<VariantMap>(source);
        }
        Variant& operator = (const std::initializer_list<VariantMap::value_type>& source) {
            emplace<VariantMap>(source);
            return *this;
        }
        // Initialize map value from integer
        template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
        Variant(const T& source) {
            emplace<Integer>(source);
        }
        template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
        Variant& operator = (const T& source) {
            emplace<Integer>(source);
            return *this;
        }
        // Initialize map value from floating-point
        template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
        Variant(const T& source) {
            emplace<Floating>(source);
        }
        template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
        Variant& operator = (const T& source) {
            emplace<Floating>(source);
            return *this;
        }
        // Initialize map value from Boolean

        Variant(const bool& source) {
            emplace<bool>(source);
        }
        Variant& operator = (const bool& source) {
            emplace<bool>(source);
            return *this;
        }

        // for debugging

        void show(std::ostream& buffer, const std::string& indent="", const bool start_with_indent=true) const {
            if (start_with_indent) {
                buffer << indent;
            }
            buffer << "[" << get_type_name() << "] ";
            switch (get_type()) {
                case Undefined:
                case Null:
                    buffer << '\n';
                    break;
                case Vector:
                    buffer << get<VariantVector>().size() << '\n';
                    for (const Variant& value : get<VariantVector>()) {
                        buffer << indent << "  • ";
                        value.show(buffer, indent + "    ", false);
                    }
                    break;
                case Map:
                    buffer << get<VariantMap>().size() << '\n';
                    for (const std::pair<std::string, Variant>& key_value : get<VariantMap>()) {
                        buffer << indent << "  • " << key_value.first << " ⇢ ";
                        key_value.second.show(buffer, indent + "    ", false);
                    }
                    break;
                default:
                    std::visit(ShowVisitor(buffer), *(VariantBase*)this);
                    buffer << '\n';
            }
        }

        #define TYPES__VARIANT__MAKE_GETTER_SETTER(NAME, TYPE, ENUMTYPE) \
            const TYPE& get_##NAME() const { return get<TYPE>(); } \
            const bool is_##NAME() const { return index() == ENUMTYPE; } \
            TYPE& get_##NAME() { return get<TYPE>(); } \
            void set_##NAME() { emplace<TYPE>(); } \
            void set_##NAME(const TYPE& value) { emplace<TYPE>(value); }
        TYPES__VARIANT__MAKE_GETTER_SETTER(boolean, bool, Boolean)
        TYPES__VARIANT__MAKE_GETTER_SETTER(character, char, Character)
        TYPES__VARIANT__MAKE_GETTER_SETTER(integer, int64_t, Integer)
        TYPES__VARIANT__MAKE_GETTER_SETTER(floating, double, Floating)
        TYPES__VARIANT__MAKE_GETTER_SETTER(string, std::string, String)
        TYPES__VARIANT__MAKE_GETTER_SETTER(vector, VariantVector, Vector)
        TYPES__VARIANT__MAKE_GETTER_SETTER(map, VariantMap, Map)
        TYPES__VARIANT__MAKE_GETTER_SETTER(datetime, Types::DateTime, DateTime)
        #undef TYPES__VARIANT__MAKE_GETTER_SETTER

        const bool is_null() const {
            return index() == Null;
        }

        const double get_number() const {
            switch (get_type()) {
                case Integer:
                    return (const double) get_integer();
                case Floating:
                    return (const double) get_floating();
                case String:
                    return std::stod(get_string());
                default:
                    throw Exceptions::Exception("You tried to read this Variant as a number, but it is in fact a " + get_type_name());
            }
        }

        void unset() {
            emplace<VariantNull>();
        }

    private:

        class ShowVisitor {
        public:
            ShowVisitor(std::ostream& buffer) : _buffer(buffer) {}
            void operator() (const auto& value) { _buffer << value; }
            void operator() (const bool& value) { _buffer << (value ? "true" : "false"); }
            void operator() (const VariantNull& value) { _buffer << "null"; }
            void operator() (const VariantMap& value) { }
            void operator() (const VariantVector& value) { }
        private:
            std::ostream& _buffer;
        };

        static const std::string _undefined_type_name;
        static const std::vector<std::string> _type_names;
        static const Variant _undefined;

    };

    const std::string Variant::_undefined_type_name = "Undefined";
    const std::vector<std::string> Variant::_type_names = {
        "Null",
        "Boolean",
        "Character",
        "Integer",
        "Floating",
        "String",
        "Vector",
        "Map",
        "DateTime",
    };
    const Variant Variant::_undefined;

    std::ostream& operator<< (std::ostream& buffer, const Variant& variant) {
        variant.show(buffer);
        return buffer;
    }


} // Types


#endif // CPPP____INCLUDE____TYPES__VARIANT_HPP
