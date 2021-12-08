#include "Compression/Gzip.hpp"


int main(int argc, char const *argv[]) {
    Compression::Gzip compressor;
    std::string lipsum = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.";
    std::string compressed, expanded;
    //
    compressor.compress(compressed, lipsum);
    std::cout << compressed << '\n';
    //
    FILE* f = fopen("/tmp/test.txt.gz", "wb");
    fwrite(compressed.data(), 1, compressed.size(), f);
    fclose(f);
    //
    compressor.expand(expanded, compressed);
    std::cout << expanded << '\n';
    //
    return 0;
}
