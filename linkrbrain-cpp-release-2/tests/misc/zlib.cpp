#include "Compression/Zlib/OStream.hpp"
#include "Compression/Zlib/IStream.hpp"
#include "Generators/NumberName.hpp"

#include <fstream>


static const size_t N = 1<<20;


int main(int argc, char const *argv[]) {
    // deflate
    {
        std::ifstream input("data/disorders/database_disorderdb.csv");
        std::ofstream output("/tmp/database_disorderdb.csv.gz");
        Compression::Zlib::OStream gz_output(output);
        for (size_t i = 0; i < N; i++) {
            gz_output << (char)('A' + i%26) << ' ' << Generators::NumberName::get_english_name(i) << '\n';
            gz_output.write("azertyuiopqsdfghjklmwxcvbn\n", 27);
        }
    }
    // inflate
    {
        std::ifstream gz_input("/tmp/database_disorderdb.csv.gz");
        std::ofstream output("/tmp/database_disorderdb.csv");
        Compression::Zlib::IStream input(gz_input);
        // output << input;
        std::copy(
            std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>(),
            std::ostreambuf_iterator<char>(output));
    }
    return 0;
}
