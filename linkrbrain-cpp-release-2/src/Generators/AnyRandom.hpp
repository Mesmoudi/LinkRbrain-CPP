#ifndef LINKRBRAIN2019__SRC__GENERATORS__ANYRANDOM_HPP
#define LINKRBRAIN2019__SRC__GENERATORS__ANYRANDOM_HPP


#include "./Random.hpp"
#include "./NumberName.hpp"

#include "Types/FixedString.hpp"
#include "Types/VarString.hpp"


namespace Generators {

    class AnyRandom {
    public:

        AnyRandom() :
            numbername_min(0),
            numbername_max(+5000),
            boolean_frequency(0.5) {}

        template <typename T>
        void randomize(T& value) const {
            value = Random::generate<T>();
        }
        void randomize(bool& value) const {
            value = (Random::generate_number(1.f) < boolean_frequency);
        }
        void randomize(std::string& value) const {
            const int64_t number = Random::generate_number(numbername_min, numbername_max);
            value = NumberName::get_english_name(number);
        }
        template <size_t N>
        void randomize(Types::FixedString<N>& value) const {
            std::string source;
            randomize(source);
            value = source;
        }
        void randomize(void* data, size_t size) const {
            uint32_t* block = (uint32_t*) data;
            const size_t n = size / sizeof(*block);
            for (size_t i=0; i<n; ++i) {
                block[i] = std::rand();
            }
        }

        template <typename T>
        const T generate_random() const {
            T value;
            randomize(value);
            return value;
        }

        int64_t numbername_min;
        int64_t numbername_max;
        float boolean_frequency;

    };

} // Generators


#endif // LINKRBRAIN2019__SRC__GENERATORS__ANYRANDOM_HPP
