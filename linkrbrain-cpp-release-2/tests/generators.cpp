#include "Generators/Random.hpp"
#include "Generators/NumberName.hpp"
#include "Generators/RandomVariant.hpp"

#include <iostream>

#include "Logging/Logger.hpp"


const static int N = 20;


int main(int argc, char const *argv[]) {

    auto& logger = Logging::get_logger();
    Generators::Random::reseed(2);

    logger.message("Test random with uint16_t");
    for (int i=0; i<N; ++i) {
        logger.debug(Generators::Random::generate<uint16_t>());
    }
    logger.message("Test random with int64_t");
    for (int i=0; i<N; ++i) {
        logger.debug(Generators::Random::generate<int64_t>());
    }
    logger.message("Test random with double");
    for (int i=0; i<N; ++i) {
        logger.debug(Generators::Random::generate<double>());
    }
    logger.message("Test number namer");
    logger.debug(Generators::NumberName::get_english_name(-416));
    logger.debug(Generators::NumberName::get_english_name(-321000416));
    logger.debug(Generators::NumberName::get_english_name(-321001416));
    const auto number = Generators::Random::generate<int64_t>();
    logger.debug(number, "is", Generators::NumberName::get_english_name(number));
    logger.message("Test random with variant");
    Buffering::Writing::OStreamWriter writer(std::cout, Buffering::Format::JSON);
    for (int i=0; i<N; ++i) {
        const Types::Variant v = Generators::RandomVariant::generate();
        logger.debug(v);
        writer << v;
        writer.write("\n");
    }

    return 0;
}
