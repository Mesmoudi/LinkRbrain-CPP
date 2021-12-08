#include "Utils/Logger.hpp"
#include "Reading/CSVReader.hpp"

#include <iostream>
#include <vector>


const std::string path = "data/old/genes/SampleAnnot.csv";
const size_t skip_lines = 1;


int main(int argc, char const *argv[]) {

    Logger logger(std::cerr, true);
    logger.notice("instanciated logger");
    Reading::CSVReader csv(path, skip_lines);
    logger.notice("instanciated reader");

    std::vector<std::string> values;
    while (csv.parse_line(values)) {
        std::cout << values[12] << '\n';
    }
    logger.notice("read whole file");
    return 0;
}
