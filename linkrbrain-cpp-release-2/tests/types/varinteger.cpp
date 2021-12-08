#include <iostream>
#include <vector>

#include "Types/VarInteger.hpp"
#include "Generators/Random.hpp"
#include "Logging/Logger.hpp"

#include "Buffering/Writing/StringWriter.hpp"
#include "Buffering/Reading/StringReader.hpp"
#include "Buffering/Writing/FileWriter.hpp"
#include "Buffering/Reading/FileReader.hpp"


static const size_t n = 5e6;
auto& logger = Logging::get_logger();


int main(int argc, char const *argv[]) {
    // Generators::Random::reseed(0);
    logger.notice(sizeof(Types::VarUInt64), "bytes per VarInteger");
    for (int i=0; i<n; ++i) {
        int64_t source = Generators::Random::generate<int64_t>();
        Types::VarUInt64 varint(source);
        int64_t destination = varint.get_integer();
        if (source != destination) {
            logger.error("Error:", source, " became ", destination);
            return 1;
        }
    }
    logger.notice("Generated and checked", n, "numbers");
    std::vector<int64_t> values;
    for (int i=0; i<n; ++i) {
        values.push_back(
            i
        );
        values.push_back(
            Generators::Random::generate_number(1e6)
        );
    }
    logger.notice("Generated", values.size(), "numbers");
    // Buffering::Writing::StringWriter writer(Buffering::Format::Binary);
{
    Buffering::Writing::FileWriter writer("/tmp/numbers.buffer", Buffering::Format::Binary);
    for (const int64_t source : values) {
        Types::VarInt64 destination(source);
        writer << destination;
    }
    logger.notice("Wrote", values.size(), "varintegers");
    logger.debug("Size is", writer.get_written_size(), "instead of", values.size()*sizeof(uint32_t), "or", values.size()*sizeof(uint64_t));
}
    // Buffering::Reading::StringReader reader(writer.get_string());
    Buffering::Reading::FileReader reader("/tmp/numbers.buffer");
    size_t i = 0;
    for (const auto& value : values) {
        if (values[i] != value) {
            except("what the.", value, values[i]);
        }
        ++i;
    }
    logger.notice("Read and checked", i, "normal integers");
    i = 0;
    while (!reader.is_finished() && i < values.size()) {
        Types::VarInt64 destination(reader);
        if (values[i] != destination.get_integer()) {
            logger.warning("at #", i, "failed to identify", values[i], "with", destination.get_integer());
            logger.warning("cursor is at", reader.get_read_size());
            break;
        }
        ++i;
    }
    logger.notice("Read and checked", i, "varintegers");

    return 0;
}
