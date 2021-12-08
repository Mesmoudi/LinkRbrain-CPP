#include "Generators/NumberName.hpp"
#include "Generators/Random.hpp"
#include "Types/Variant.hpp"
#include "Exceptions/Exception.hpp"
#include "Logging/Loggers.hpp"
auto& logger = Logging::get_logger();


typedef std::unordered_map<std::string, Types::Variant> Dict;
static const size_t keys_count = 1<<4;
static const size_t N = 1<<22;
static const size_t tests_count = 2;


int retrieve1(const std::string& key, const Dict& dict) {
    int s = 0;
    for (size_t i=0; i<N; ++i) {
        auto it = dict.find(key);
        if (it != dict.end()) {
            s += it->second.get_type();
        }
    }
    return s;
}
int retrieve2(const std::string& key, const std::vector<std::string> keys, const std::vector<Types::Variant> values) {
    int s = 0;
    for (size_t i=0; i<N; ++i) {
        for (size_t j=0, m=keys.size(); j<m; ++j) {
            if (keys[j] == key) {
                s += values[j].get_type();
                break;
            }
        }
    }
    return s;
}


int main(int argc, char const *argv[]) {

    // make data
    const std::vector<Types::Variant> all_values = {
        42,
        "My request",
        {{"this","that"}, {"is_something",true}, {"pi",3.14159}},
        Types::DateTime::now()
    };
    std::vector<std::string> keys;
    std::vector<Types::Variant> values;
    for (size_t i=0; i<keys_count; ++i) {
        keys.push_back(
            Generators::NumberName::get_english_name(i)
        );
        values.push_back(
            Generators::Random::pick(all_values)
        );
    }
    logger.notice("Keys and values are ready");
    Dict dict;
    for (int i=0, n=keys.size(); i<n; ++i) {
        dict[keys[i]] = values[i];
        Buffering::Writing::StringWriter buffer(Buffering::Format::JSON);
        buffer << Types::Variant({{keys[i], values[i]}});
        if (i < 10 || i == n-1) {
            logger.debug("#", i, "\t", buffer.get_string());
        }
    }
    logger.message("Data is ready");

    for (size_t i=0; i<tests_count; ++i) {
        const std::string& key = keys[i % keys.size()];
        logger.debug("Tested unordered_map, checksum is", retrieve1(key, dict));
        logger.debug("Tested vectors, checksum is", retrieve2(key, keys, values));
        logger.notice("Tested both methods with key", key);
    }
    {
        const std::string& key = keys[keys.size() / 2];
        logger.debug("Tested unordered_map, checksum is", retrieve1(key, dict));
        logger.debug("Tested vectors, checksum is", retrieve2(key, keys, values));
        logger.notice("Tested both methods with key", key);
    }
    for (size_t i=0; i<tests_count; ++i) {
        const std::string key = keys[(keys.size() - tests_count + i) % keys.size()];
        logger.debug("Tested unordered_map, checksum is", retrieve1(key, dict));
        logger.debug("Tested vectors, checksum is", retrieve2(key, keys, values));
        logger.notice("Tested both methods with key", key);
    }
    logger.message("Tested both methods");
    return 0;
}
