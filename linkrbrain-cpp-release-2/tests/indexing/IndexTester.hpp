#include "Indexing/Index.hpp"
#include "Indexing/BTree.hpp"
#include "Indexing/Trie.hpp"
#include "Indexing/Bitmap.hpp"
#include "Indexing/FixedPrimary.hpp"
#include "Generators/AnyRandom.hpp"
#include "Logging/Loggable.hpp"
#include "Mapped/Directory.hpp"

#include <vector>
#include <map>


typedef uint32_t btree_page_index_t;
typedef uint16_t btree_onpage_index_t;
typedef uint8_t trie_keysize_t;
typedef uint32_t trie_pageindex_t;
typedef uint16_t trie_size_t;


template <typename ...Pairs>
class IndexTester : public Logging::Loggable {
public:

    IndexTester(Mapped::Directory& directory) :
        _directory(directory),
        _index_counter(0) {}

    template <typename Key, typename Value>
    void test_index(const std::string& name, Indexing::Index<Key, Value>& index, const std::vector<std::pair<Key, Value>>& pairs, const std::map<Key, Value>& ordered_pairs) {
        // insertions
        size_t inserted_values_count = 0;
        for (const auto& [key, value] : ordered_pairs) {
            ++inserted_values_count;
            index.insert(key, value);
        }
        get_logger().debug(name, "index: inserted", inserted_values_count, "values");
        // equality check, part one
        for (const auto& [key, value] : ordered_pairs) {
            const Value index_value = index.get(key);
            if (index_value != value) {
                get_logger().warning(name, "index: values mismatch: `", std::to_string(value), "` and `", std::to_string(index_value), "`");
            }
        }
        get_logger().debug(name, "index: performed equality test (browsing", ordered_pairs.size(), "elements from map)");
        // // equality check, part two
        // size_t index_elements_count = 0;
        // for (const auto& [key, value] : index.all()) {
        //     const auto result = ordered_pairs.find(key);
        //     if (result == ordered_pairs.end()) {
        //         get_logger().warning(name, "index: key in index: `", std::to_string(key), "`");
        //     }
        //     if (result->second != value) {
        //         get_logger().warning(name, "index: values mismatch for key", key, ": `", std::to_string(value), "` was found in index, should be `", std::to_string(result->second), "`");
        //     }
        //     ++index_elements_count;
        // }
        // if (index_elements_count != ordered_pairs.size()) {
        //     get_logger().warning(name, "index: count mismatch: index has", index_elements_count, "values, but there should be", ordered_pairs.size());
        // }
        // get_logger().debug(name, "index: performed second equality test (browsing", index_elements_count, "elements from index)");
    }

    template <typename Key, typename Value>
    Indexing::Index<Key, Value>* make_trie() {
        return new Indexing::Trie<
            Key, trie_keysize_t, Value,
            trie_pageindex_t, trie_size_t, sizeof(Key)
        >(
            _directory, get_index_name()
        );
    }
    template <typename Key, typename Value>
    Indexing::Index<Key, Value>* make_btree() {
        return new Indexing::BTree<
            Key, Value,
            btree_page_index_t, btree_onpage_index_t
        >(
            _directory, get_index_name()
        );
    }
    template <typename Key, typename Value, std::enable_if_t<!std::is_same<Value, bool>::value, int> = 0>
    Indexing::Index<Key, Value>* make_bitmap() {
        return NULL;
    }
    template <typename Key, typename Value, std::enable_if_t<std::is_same<Value, bool>::value, int> = 0>
    Indexing::Index<Key, Value>* make_bitmap() {
        return new Indexing::Bitmap<
            Key
        >(
            _directory, get_index_name()
        );
    }
    template <typename Key, typename Value, std::enable_if_t<!std::is_convertible<Key, size_t>::value, int> = 0>
    Indexing::Index<Key, Value>* make_primary() {
        return NULL;
    }
    template <typename Key, typename Value, std::enable_if_t<std::is_convertible<Key, size_t>::value, int> = 0>
    Indexing::Index<Key, Value>* make_primary() {
        return new Indexing::FixedPrimary<
            Key, Value
        >(
            _directory, get_index_name()
        );
    }

    template <typename Pair>
    void test(const size_t n) {
        typedef typename Pair::first_type Key;
        typedef typename Pair::second_type Value;
        get_logger().debug("Start test:", n, "values, key type is `", typeid(typename Pair::first_type).name(), "`, value type is `", typeid(typename Pair::second_type).name(), "`");
        std::vector<Pair> pairs;
        std::map<Key, Value> ordered_pairs;
        for (size_t i = 0; i < n; i++) {
            Pair pair;
            generator.randomize(pair.first);
            generator.randomize(pair.second);
            pairs.push_back(pair);
            ordered_pairs.insert(pair);
        }
        get_logger().notice("Generated", pairs.size(), "vector values,", ordered_pairs.size(), "map values");
        get_logger().debug("First key is: `", pairs[0].first, "`");
        get_logger().debug("First value is: `", pairs[0].second, "`");
        //
        std::vector<std::pair<std::string, Indexing::Index<Key, Value>*>> indexes = {
            {"BTree", make_btree<Key, Value>()},
            {"Bitmap", make_bitmap<Key, Value>()},
            {"Trie", make_trie<Key, Value>()},
            {"Primary", make_primary<Key, Value>()},
        };
        size_t indexes_count = 0;
        for (const auto& [name, index] : indexes) {
            if (index != NULL) {
                ++indexes_count;
            }
        }
        get_logger().notice("Instanciated", indexes_count, "index(es)");
        size_t tested_indexes_count = 0;
        for (const auto& [name, index] : indexes) {
            if (index != NULL) {
                ++tested_indexes_count;
                test_index(name, *index, pairs, ordered_pairs);
                delete index;
                get_logger().debug(name, "index: deleted instance");
            }
        }
        get_logger().message("Tested", tested_indexes_count, "index(es) for this set of values");
    }

    void test(const size_t n) {
        test_helper<void, Pairs...>(n);
    }

    Generators::AnyRandom generator;

protected:

    const std::string& get_logger_name() const {
        static const std::string name = "IndexTester";
        return name;
    }

private:

    const std::string get_index_name() {
        return Generators::NumberName::get_english_name(++_index_counter);
    }

    template <typename Nothing>
    void test_helper(const size_t n) {
    }
    template <typename Nothing, typename Pair, typename ...OtherPairs>
    void test_helper(const size_t n) {
        test<Pair>(n);
        test_helper<Nothing, OtherPairs...>(n);
    }

    size_t _index_counter;
    Mapped::Directory& _directory;
};
