#include <iostream>
#include <unordered_map>

#include "Types/Variant.hpp"
#include "Testing/Test.hpp"


static const size_t N = 1<<10;


class Test : public Testing::Test {
public:

    Test() {
        get_logger().set_level(Logging::Debug);
    }

    void test() {
        test_simple();
        test_conversion();
        test_operations();
        test_performance_map();
        test_formats();
    }

    void test_formats() {
        get_logger().message("Test formats");
        test_format_json();
        test_format_binary();
    }
    void test_format_json() {
        const std::string json = Formats::to_json(v);
        get_logger().debug("Converted to JSON:\n", json);
        const Types::Variant v2 = Formats::from_json<Types::Variant>(json);
        get_logger().debug("Back to Variant:\n", v2);
        ASSERT_EQ(v, v2);
        get_logger().notice("It matches!");
    }
    void test_format_binary() {
        const std::string binary = Formats::to_binary(v);
        get_logger().debug("Converted to binary:\n", binary);
        const Types::Variant v2 = Formats::from_binary<Types::Variant>(binary);
        get_logger().debug("Back to Variant:\n", v2);
        ASSERT_EQ(v, v2);
        get_logger().notice("It matches!");
    }


    void test_performance_map() {
        get_logger().message("Start testing map performance for uint8_t");
        test_performance_map<uint8_t>();
        get_logger().message("Start testing map performance for int64_t");
        test_performance_map<int32_t>();
        get_logger().message("Start testing map performance for int64_t");
        test_performance_map<int64_t>();
    }
    void test_performance_vector() {
        get_logger().message("Start testing map performance for uint8_t");
        test_performance_map<uint8_t>();
        get_logger().message("Start testing map performance for int64_t");
        test_performance_map<int32_t>();
        get_logger().message("Start testing map performance for int64_t");
        test_performance_map<int64_t>();
    }

    template<class T, std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>
    void test_performance_map() {
        // generate values
        std::vector<std::pair<std::string, T>> values;
        std::vector<std::pair<Types::Variant, Types::Variant>> variant_values;
        for (int i=0; i<N; ++i) {
            const T value = generate<T>();
            // const T value = i;
            values.push_back({Generators::NumberName::get_english_name(value), value});
            variant_values.push_back({Generators::NumberName::get_english_name(value), value});
            // stdout << "`" << Generators::NumberName::get_english_name(value) << "`\n";
        }
        get_logger().debug("Generated ", values.size(), " values, both native and Variant");
        // test
        std::map<std::string, T> map;
        for (const auto& value : values) {
            map.insert(value);
        }
        get_logger().notice("Integrated ", map.size(), " values into map");
        for (const auto& [key, value] : values) {
            const auto result = map.find(key);
            ASSERT(result != map.end());
            ASSERT_EQ(result->second, value);
        }
        get_logger().debug("Compared ", map.size(), " values from map");
        //
        std::unordered_map<std::string, T> unordered_map;
        for (const auto& value : values) {
            unordered_map.insert(value);
        }
        get_logger().notice("Integrated ", unordered_map.size(), " values into unordered_map");
        for (const auto& [key, value] : values) {
            const auto result = unordered_map.find(key);
            ASSERT(result != unordered_map.end());
            ASSERT_EQ(result->second, value);
        }
        get_logger().debug("Compared ", unordered_map.size(), " values from unordered_map");
        //
        Types::Variant variant;
        for (const auto& [key, value] : variant_values) {
            variant[key.get<std::string>()] = value;
        }
        get_logger().notice("Integrated ", variant.get<Types::VariantMap>().size(), " values into variant map");
        for (const auto& [key, value] : values) {
            ASSERT_EQ(variant[key], value);
        }
        get_logger().debug("Compared ", variant.get<Types::VariantMap>().size(), " values from variant map");
    }

    template <typename T>
    void test_performance_vector() {
        // generate values
        std::vector<std::pair<std::string, T>> values;
        std::vector<std::pair<Types::Variant, Types::Variant>> variant_values;
        for (int i=0; i<N; ++i) {
            const T value = generate<T>();
            // const T value = i;
            values.push_back({Generators::NumberName::get_english_name(value), value});
            variant_values.push_back({Generators::NumberName::get_english_name(value), value});
            // stdout << "`" << Generators::NumberName::get_english_name(value) << "`\n";
        }
        get_logger().debug("Generated ", values.size(), " values");
        //
        size_t i;
        //
        std::vector<std::string> vector_string;
        std::vector<T> vector_integer;
        for (const auto& [key, value] : values) {
            vector_string.push_back(key);
            vector_integer.push_back(value);
        }
        get_logger().notice("Integrated ", vector_string.size() + vector_integer.size(), " values into vectors");
        i = 0;
        for (const auto& [key, value] : values) {
            ASSERT_EQ(vector_string[i], key);
            ASSERT_EQ(vector_integer[i], value);
            ++i;
        }
        get_logger().debug("Compared ", vector_string.size() + vector_integer.size(), " values from variant vector");
        //
        Types::Variant variant;
        for (const auto& [key, value] : values) {
            variant.push_back(key);
            variant.push_back(value);
        }
        get_logger().notice("Integrated ", variant.size(), " values into variant vector");
        i = 0;
        for (const auto& [key, value] : variant_values) {
            ASSERT_EQ(variant[i], key);
            i++;
            ASSERT_EQ(variant[i], value);
            i++;
        }
        get_logger().debug("Compared", variant.size(), "values from variant vector");
    }

    void test_conversion() {
        get_logger().message("Starting conversion test");
        Types::Variant value1 = {{"key", 123}, {"something", "else"}};
        test_value(value1);
        Types::Variant value2 = {"test1", "test2", "test3"};
        test_value(value2);
        Types::Variant value3 = {3.14, 159.26, 26537.};
        test_value(value3);
    }

    void test_operations() {
        get_logger().message("Starting operations tests");
        v["a"] = "aa";
        v["b"] = {"b0", "b1", "b2"};
        v["k"].emplace<Types::VariantVector>();
        v["k"].get<Types::VariantVector>().push_back("test");
        v["j"] = {"a", "b", "c"};
        v["empty"];
        std::cout << v << '\n';
        ASSERT(v.contains("e"));
        ASSERT(!v.contains("vector"));
        ASSERT(!v.contains("vectors"));
        v["a_true_boolean"] = true;
        v["a_false_thing"] = false;
        v["test"] = 8;
        v["truc"] = "bidule";
        v["truc"] = v["machin"] = "bidule";
        v["ints"] = {1, 2, 3, 4};
        v["ints_as_strings"] = {"1", "2", "3", "4"};
        v["ghi"] = {{"a", 1}, {"b", 2}, {"c", 3}};
        v["ghi"]["d"] = {"this", "is", "a", "vector"};
        ASSERT(v.contains("vector"));
        ASSERT(!v.contains("vectors"));
        ASSERT_EQ(v["ghi"]["a"], 1);
        std::cout << v << '\n';
    }

    void test_simple() {
        get_logger().message("Starting simple tests");
        test_value(123L);
        test_value(123.4f);
        test_value('a');
        test_value((uint8_t)1);
        test_value((int8_t)1);
        test_value("Hello world!");
    }

    template <typename T>
    void test_value(const T& source) {
        get_logger().notice("original value =\n", source);
        Types::Variant value(source);
        get_logger().debug("Value type index = ", value.get_type());
        get_logger().debug("Value type name = ", value.get_type_name());
    }

protected:

    virtual const std::string get_logger_name() {
        return "Test for Types::Variant";
    }

private:

    Types::Variant v;

};


int main(int argc, char const *argv[]) {
    Test().test();
    return 0;
}
