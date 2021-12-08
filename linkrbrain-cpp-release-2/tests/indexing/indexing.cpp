#include "IndexTester.hpp"
#include "Types/Integers.hpp"
#include "LinkRbrain/Models/Point.hpp"


static const size_t n = 1e5;
static const std::string path = "/tmp/dbtest";
static const size_t page_size = 4096;
static const size_t pageblock_size = 1048576;
static const size_t max_cache_size=1073741824L;


int main(int argc, char const *argv[]) {
    Mapped::Manager manager(page_size, pageblock_size, max_cache_size);
    Mapped::Directory directory(manager, path);
    //
    IndexTester<
        std::pair<uint16_t, bool>,
        std::pair<Types::FixedString<32>, uint8_t>
    > index_tester(directory);
    index_tester.test(n);

    // Types::FixedString<16> test;
    // for (size_t i = 0; i < 10; i++) {
    //     // std::cout << generator.generate_random<std::string>() << '\n';
    //     // std::cout << generator.generate_random<Types::FixedString<16>>() << '\n';
    //     // std::cout << generator.generate_random<bool>() << '\n';
    // }
    return 0;
}
