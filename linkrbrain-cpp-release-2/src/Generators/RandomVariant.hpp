#ifndef LINKRBRAIN2019LINKRBRAIN2019__SRC__GENERATORS__RANDOMVARIANT_HPP
#define LINKRBRAIN2019LINKRBRAIN2019__SRC__GENERATORS__RANDOMVARIANT_HPP


#include "./Random.hpp"
#include "./NumberName.hpp"
#include "Types/Variant.hpp"


namespace Generators::RandomVariant {

    Types::Variant generate(const std::vector<Types::Variant::Type>& types = {Types::Variant::UndefinedType, Types::Variant::NullType, Types::Variant::IntegerType, Types::Variant::FloatingType, Types::Variant::VectorType, Types::Variant::StringType, Types::Variant::MapType}, const int depth_limit=3, const size_t n=10) {
        Types::Variant result;
        Types::Variant::Type result_type = Generators::Random::pick(types);
        if (depth_limit == 0) {
            while (result_type == Types::Variant::VectorType || result_type == Types::Variant::MapType) {
                result_type = Generators::Random::pick(types);
            }
        }
        switch (result_type) {
            case Types::Variant::UndefinedType:
            case Types::Variant::NullType:
                result.set_type(result_type);
                break;
            case Types::Variant::BooleanType:
                result.set_boolean(Generators::Random::generate_number(0, 1, 1));
                break;
            case Types::Variant::IntegerType:
                result.set_integer(Generators::Random::generate<int64_t>());
                break;
            case Types::Variant::FloatingType:
                result.set_floating(Generators::Random::generate<double>());
                break;
            case Types::Variant::StringType:
                result.set_string(
                    Generators::NumberName::get_english_name(
                        Generators::Random::generate_number<size_t>(n)
                    )
                );
                break;
            case Types::Variant::VectorType: {
                result.set_vector();
                const size_t m = Generators::Random::generate_number<size_t>(n);
                result.get_vector().resize(n);
                for (size_t i=0; i<m; ++i) {
                    result.get_vector()[i] = generate(types, depth_limit-1, n);
                }
                break;
            }
            case Types::Variant::MapType:
                result.set_map();
                const size_t n = Generators::Random::generate_number<size_t>(n);
                for (size_t i=0; i<n; ++i) {
                    const std::string key = Generators::NumberName::get_english_name(
                        Generators::Random::generate_number<size_t>(n)
                    );
                    Types::Variant value = generate(types, depth_limit-1);
                    result.get_map().insert({key, value});
                }
                break;
        }
        return result;
    }

} // Generators::RandomVariant


#endif // LINKRBRAIN2019LINKRBRAIN2019__SRC__GENERATORS__RANDOMVARIANT_HPP
