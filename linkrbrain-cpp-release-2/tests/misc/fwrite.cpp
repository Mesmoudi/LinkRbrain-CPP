#include <stdio.h>
#include <string>


static const size_t written_size = 1 << 8;
static const size_t total_size = 1 << 24;


int main(int argc, char const *argv[]) {
    std::string buffer(written_size, '.');
    // first file is opened, then seeked
    FILE* f1 = fopen("/tmp/testfile1.bin", "w");
    fseek(f1, total_size, SEEK_SET);
    fclose(f1);
    // second file is opened, then seeked, then written to
    FILE* f2 = fopen("/tmp/testfile2.bin", "w");
    fseek(f2, total_size - written_size, SEEK_SET);
    fwrite(buffer.data(), buffer.size(), 1, f2);
    fclose(f2);
    // that was it
    return 0;
}
