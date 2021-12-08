#ifndef LINKRBRAIN2019__SRC__GENERATORS__RANDOM_HPP
#define LINKRBRAIN2019__SRC__GENERATORS__RANDOM_HPP


#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>


namespace Generators {

    class Random {
    public:

        template <typename T>
        static inline const T generate() {
            static const size_t r = (int) (log2(RAND_MAX) / 8);
            static const size_t n = ceil((double) sizeof(T) / r);
            union {
                T result{};
                char char_blocks[0];
                int int_blocks[(sizeof(T) / sizeof(int)) + 1];
            } tmp;
            for (int i=0, j=0; i<n; ++i, j+=r) {
                * (int*) (tmp.char_blocks + j) = rand();
            }
            return tmp.result;
        }

        template <typename T>
        static inline const T generate_number(const T min, const T max, const T step) {
            return min + step * (std::rand() % (int)((max - min) / step));
        }
        template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
        static inline const T generate_number(const T min, const T max) {
            return min + std::rand() % (max - min);
        }
        template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
        static inline const T generate_number(const T min, const T max) {
            return min + std::rand() / ((T)(RAND_MAX + 1u) / max);
        }
        template <typename T>
        static inline const T generate_number(const T max) {
            return generate_number((T)0, max);
        }

        template <typename T>
        static const T& pick(const std::vector<T>& values) {
            return values[generate_number<size_t>(0, values.size())];
        }
        static const char pick(const std::string& text) {
            return text[generate_number<size_t>(0, text.size())];
        }

        static inline void reseed() {
            reseed(std::time(NULL));
        }
        static inline void reseed(unsigned seed) {
            std::srand(seed);
        }

    };

} // Generators


#endif // LINKRBRAIN2019__SRC__GENERATORS__RANDOM_HPP
