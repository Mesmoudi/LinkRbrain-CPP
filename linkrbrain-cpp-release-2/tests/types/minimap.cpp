#include "Types/Variant.hpp"
#include "Logging/Loggers.hpp"
auto& logger = Logging::get_logger();

#include "Types/MiniMap.hpp"

#include <string>


int main(int argc, char const *argv[]) {
    Types::MiniMap<std::string, std::string> minimap;
    minimap.insert("a", "test_a_1");
    minimap.insert("b", "test_b");
    minimap.insert("a", "test_a_2");
    minimap.insert("c", "test_c");
    logger.message("Tested `insert`:", minimap.size());
    logger.debug(minimap.get_all("a"));
    logger.debug(minimap.get_all("b"));
    logger.debug(minimap.get_all("c"));
    logger.debug(minimap.get_all("d"));
    logger.message("Tested `get_all`");
    logger.debug(minimap.get("a", "fallback_a"));
    logger.debug(minimap.get("b", "fallback_b"));
    logger.debug(minimap.get("c", "fallback_c"));
    logger.debug(minimap.get("d", "fallback_d"));
    logger.message("Tested `get` with fallback");
    logger.debug(minimap.get("a"));
    logger.debug(minimap.get("b"));
    logger.debug(minimap.get("c"));
    logger.debug(minimap.get("d"));
    logger.message("Tested `get`");
    return 0;
}
