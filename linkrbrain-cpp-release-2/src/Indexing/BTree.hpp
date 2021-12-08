#ifndef __DUPADB__INDEXING__BTREE_HPP
#define __DUPADB__INDEXING__BTREE_HPP


#include "Paged/Directory.hpp"
#include "./Iteration/WrapIterator.hpp"
#include "./Iteration/BaseIterator.hpp"
#include "./Index.hpp"


namespace Indexing {

    static const uint8_t BTREE_MAX_DEPTH = 254;

    #pragma pack(push, 1)

    template<typename key_t, typename value_t, typename page_index_t, typename onpage_index_t>
    struct BTreeHeader : public Paged::Header {
        std::size_t key_size;
        std::size_t value_size;
        std::size_t page_index_size;
        std::size_t onpage_index_size;
        std::size_t next_page_index;
        bool is_initialized;
        inline void set(std::size_t page_size, const std::string& file_type) {
            Paged::Header::set(page_size, file_type);
            key_size = sizeof(key_t);
            value_size = sizeof(value_t);
            page_index_size = sizeof(page_index_t);
            onpage_index_size = sizeof(onpage_index_t);
            next_page_index = 0;
            is_initialized = false;
        }
        inline void check(std::size_t page_size, const std::string& file_type, const std::string& file_path) const {
            Paged::Header::check(page_size, file_type, file_path);
            if (key_size != sizeof(key_t))
                throw Paged::BadHeaderException(file_path, "key size", sizeof(key_t), key_size);
            if (value_size != sizeof(value_t))
                throw Paged::BadHeaderException(file_path, "value size", sizeof(value_t), value_size);
            if (page_index_size != sizeof(page_index_t))
                throw Paged::BadHeaderException(file_path, "page index size", sizeof(page_index_t), page_index_size);
            if (onpage_index_size != sizeof(onpage_index_t))
                throw Paged::BadHeaderException(file_path, "onpage index size", sizeof(onpage_index_t), onpage_index_size);
        }
    };

    template<typename key_t, typename value_t, typename page_index_t, typename onpage_index_t>
    std::ostream& operator << (std::ostream& os, const BTreeHeader<key_t, value_t, page_index_t, onpage_index_t>& header) {
        return os
            << "key size: " << header.key_size << "\n"
            << "value size: " << header.value_size << "\n"
            << "page index size: " << header.page_index_size << "\n"
            << "onpage index size: " << header.onpage_index_size << "\n"
            << "next page index: " << header.next_page_index << "\n"
            << "is_initialized: " << header.is_initialized << "\n";
    }

    template<typename key_t, typename value_t, typename page_index_t, typename onpage_index_t>
    struct BTreePage {
        bool is_root : 4;
        bool is_leaf : 4;
        onpage_index_t keys_count;
        union {
            struct {
                page_index_t page_index;
                key_t key;
            } node[0];
            struct {
                key_t key;
                value_t value;
            } leaf[0];
        };
        operator const std::string() {
            std::stringstream s;
            s << *this;
            return s.str();
        }
    };

    template<typename key_t, typename value_t, typename page_index_t, typename onpage_index_t>
    std::ostream& operator << (std::ostream& os, const BTreePage<key_t, value_t, page_index_t, onpage_index_t>& page) {
        if (page.is_leaf) {
            os
                << "<BTree"
                << (page.is_root ? " root": "")
                << " leaf - "
                << (page.keys_count + 0)
                << " keys\n";
            for (onpage_index_t i=0; i<page.keys_count; i++) {
                os << "\t#" << (i+0) << " `" << page.leaf[i].key << "` => `" << page.leaf[i].value << "`\n";
            }
        } else {
            os
                << "<BTree"
                << (page.is_root ? " root": "")
                << " node - "
                << (page.keys_count + 0)
                << " keys\n";
            for (onpage_index_t i=0; i<page.keys_count; i++) {
                os << "\t#" << (i+0) << " `" << page.node[i].key << "` => `" << page.node[i].page_index << "`\n";
            }
            os << "\t#" << (page.keys_count+0) << " => `" << page.node[page.keys_count].page_index << "`\n";
        }
        return (os << ">");
    }


    template<typename key_t, typename value_t, typename page_index_t, typename onpage_index_t>
    // struct BTree {
    struct BTree : public Index<key_t, value_t> {

        typedef BTreeHeader<key_t, value_t, page_index_t, onpage_index_t> header_t;
        typedef BTreePage<key_t, value_t, page_index_t, onpage_index_t> page_t;

        Paged::File<header_t, page_t>& _file;
        std::size_t _node_capacity;
        std::size_t _leaf_capacity;

        inline BTree(Paged::Directory& directory, const std::string path, const std::size_t page_size=0)
        : _file(directory.file<header_t, page_t>(path, "BTREE", page_size))
        , _node_capacity((_file._page_size - sizeof(onpage_index_t) - sizeof(page_index_t) - 1) / (sizeof(page_index_t) + sizeof(key_t)))
        , _leaf_capacity((_file._page_size - sizeof(onpage_index_t) - 1) / (sizeof(value_t) + sizeof(key_t)))
        {
            auto header = _file.header();
            if (!header->is_initialized) {
                header->set(_file._page_size, "BTREE");
                header->is_initialized = true;
                _file.resize(2 * _file._page_size);
                new_root_index();
            }
        }


        inline const page_index_t new_page_index() {
            auto header = _file.header();
            // std::cout << "_" << header->next_page_index << "_" << std::endl;
            const std::size_t page_index = header->next_page_index;
            // std::cout << *header << std::endl;
            header->next_page_index++;
            // std::cout << "_" << page_index << "_" << std::endl;
            // std::cout << *header << std::endl;
            return page_index;
        }

        inline const page_index_t new_root_index() {
            page_index_t root_index = new_page_index();
            auto page = _file.page(root_index);
            page->is_leaf = true;
            page->is_root = true;
            page->keys_count = 0;
            return root_index;
        }

        inline void split(const page_index_t page_index, page_index_t parent_page_index) {

            auto page = _file.page(page_index);
            const onpage_index_t split_left = page->keys_count / 2;
            const onpage_index_t split_right = page->keys_count - split_left;
            // std::cout << page_index << " : " << *page << std::endl;
            if (page->is_leaf) {
                const key_t& split_key = page->leaf[split_left].key;
                if (page->is_root) {
                    #if BTREE_DEBUG
                    // std::cout << *page << std::endl;
                    // std::cout << *this << std::endl;
                    Exceptions::Exception(MESSAGE, "SPLIT A on %zu", page_index);
                    #endif
                    // first new child leaf
                    page_index_t child1_index = new_page_index();
                    auto child1 = _file.page(child1_index);
                    child1->is_root = false;
                    child1->is_leaf = true;
                    child1->keys_count = split_left + 1;
                    memcpy(child1->leaf, page->leaf, (split_left + 1) * (sizeof(key_t) + sizeof(value_t)));
                    // second new child leaf
                    page_index_t child2_index = new_page_index();
                    auto child2 = _file.page(child2_index);
                    child2->is_root = false;
                    child2->is_leaf = true;
                    child2->keys_count = split_right - 1;
                    memcpy(child2->leaf, page->leaf + (split_left + 1), (split_right - 1) * (sizeof(key_t) + sizeof(value_t)));
                    // original
                    page->is_leaf = false;
                    page->keys_count = 1;
                    page->node[0].page_index = child1_index;
                    page->node[0].key = split_key;
                    page->node[1].page_index = child2_index;
                } else {
                    #if BTREE_DEBUG
                    // std::cout << *this << std::endl;
                    // std::cout << page_index << ": " << *page << std::endl << std::endl;
                    Exceptions::Exception(MESSAGE, "SPLIT B on %zu", page_index);
                    #endif
                    // new sibling leaf
                    page_index_t sibling_index = new_page_index();
                    auto sibling = _file.page(sibling_index);
                    sibling->is_root = false;
                    sibling->is_leaf = true;
                    sibling->keys_count = (split_right - 1);
                    memcpy(sibling->leaf, page->leaf + (split_left + 1), (split_right - 1) * (sizeof(key_t) + sizeof(value_t)));
                    // parent
                    auto parent = _file.page(parent_page_index);
                    page_node_insertat(*parent, page_node_find(*parent, split_key), split_key, sibling_index);
                    // original page
                    page->keys_count = (split_left + 1);
                }
            } else {
                const key_t& split_key = page->node[split_left].key;
                if (page->is_root) {
                    #if BTREE_DEBUG
                    // std::cout << *page << std::endl;
                    // std::cout << *this << std::endl;
                    Exceptions::Exception(MESSAGE, "SPLIT C on %zu", page_index);
                    #endif
                    // first new child node
                    page_index_t child1_index = new_page_index();
                    auto child1 = _file.page(child1_index);
                    child1->is_leaf = false;
                    child1->is_root = false;
                    child1->keys_count = split_left;
                    memcpy(child1->node, page->node, split_left * (sizeof(key_t) + sizeof(page_index_t)) + sizeof(page_index_t));
                    // second new child node
                    page_index_t child2_index = new_page_index();
                    auto child2 = _file.page(child2_index);
                    child2->is_leaf = false;
                    child1->is_root = false;
                    child2->keys_count = split_right - 1;
                    memcpy(child2->node, page->node + split_left + 1, (split_right - 1) * (sizeof(key_t) + sizeof(page_index_t)) + sizeof(key_t));
                    // original
                    page->keys_count = 1;
                    page->node[0].page_index = child1_index;
                    page->node[0].key = split_key;
                    page->node[1].page_index = child2_index;
                } else {
                    #if BTREE_DEBUG
                    // std::cout << *page << std::endl;
                    // std::cout << *this << std::endl;
                    Exceptions::Exception(MESSAGE, "SPLIT D on %zu", page_index);
                    #endif
                    // new sibling node
                    page_index_t sibling_index = new_page_index();
                    auto sibling = _file.page(sibling_index);
                    sibling->is_root = false;
                    sibling->is_leaf = false;
                    sibling->keys_count = split_right - 1;
                    memcpy(sibling->node, page->node + split_left + 1, (split_right - 1) * (sizeof(key_t) + sizeof(page_index_t)) + sizeof(key_t));
                    // original
                    page->keys_count = split_left;
                    // parent
                    auto parent = _file.page(parent_page_index);
                    page_node_insertat(*parent, page_node_find(*parent, split_key), split_key, sibling_index);
                }
            }
            #if BTREE_DEBUG
            Exceptions::Exception(DEBUG, "splitted");
            #endif
        }

        inline void insert(const key_t& key, const value_t& value, const page_index_t root_index) {
            uint8_t depth = 0;
            page_index_t page_index = root_index;
            page_index_t parent_page_index = root_index;
            auto header = _file.header();
            while (depth++ < BTREE_MAX_DEPTH) {
                auto page = _file.page(page_index);
                if (page_isfull(*page)) {
                    split(page_index, parent_page_index);
                    parent_page_index = page_index = root_index;
                    depth = 0;
                    continue;
                }
                if (page->is_leaf) {
                    onpage_index_t onpage_index = page_leaf_find(*page, key);
                    return page_leaf_insertat(*page, onpage_index, key, value);
                }
                parent_page_index = page_index;
                page_index = page->node[page_node_find(*page, key)].page_index;
                if (page_index == root_index || page_index >= header->next_page_index) {
                    if (page_index == root_index) {
                        except("(page_index == root_index == %zu)", root_index);
                    } else {
                        except("(page_index == %zu) && (header->next_page_index == %zu)", page_index, header->next_page_index);
                    }
                }
            }
            throw Indexing::KeyNotFoundException(_file, key, "reached maximum depth during insertion");
        }
        void insert(const key_t& key, const value_t& value) {
            insert(key, value, 0);
        }

        inline const value_t get(const key_t& key, const page_index_t root_index) {
            uint8_t depth = 0;
            page_index_t page_index = root_index;
            while (depth++ < BTREE_MAX_DEPTH) {
                auto page = _file.page(page_index);
                // Exceptions::Exception(MESSAGE, "page %zu", page_index);
                // std::cout << *page << std::endl << std::endl;
                if (page->is_leaf) {
                    return page_leaf_get(*page, key);
                }
                page_index = page->node[page_node_find(*page, key)].page_index;
            }
            throw Indexing::KeyNotFoundException(_file, key, "reached maximum depth during retrieval");
        }
        const value_t get(const key_t& key) {
            return get(key, 0);
        }

        inline value_t& get_reference(const key_t& key, const value_t& defaultvalue) {
            uint8_t depth = 0;
            page_index_t page_index = 0;
            page_index_t parent_page_index = 0;
            while (depth++ < BTREE_MAX_DEPTH) {
                auto page = _file.page(page_index);
                if (page_isfull(*page)) {
                    split(page_index, parent_page_index);
                    parent_page_index = page_index = 0;
                    depth = 0;
                    continue;
                }
                if (page->is_leaf) {
                    onpage_index_t onpage_index = page_leaf_find(*page, key);
                    if (page->leaf[onpage_index].key != key) {
                        page_leaf_insertat(*page, onpage_index, key, defaultvalue);
                    }
                    return page->leaf[onpage_index].value;

                }
                page_index = page->node[page_node_find(*page, key)].page_index;
            }
            throw Indexing::KeyNotFoundException(_file, key, "reached maximum depth during retrieval of reference");
        }

        // iterator
        template<
            bool has_start_value, bool has_start_condition, typename start_condition,
            bool must_check_to_continue, typename continue_condition
        >
        struct RangeIterator : Iteration::BaseIterator<value_t> {
            typedef BTree<key_t, value_t, page_index_t, onpage_index_t> btree_t;
            typedef std::pair<key_t, value_t> pair_t;
            btree_t& _btree;
            key_t _key1, _key2;
            // internals
            page_index_t _page_index;
            onpage_index_t _onpage_index;
            std::vector<page_index_t> _page_index_path;
            std::vector<onpage_index_t> _onpage_index_path;
            // interesting values
            bool _is_iterable;
            bool _is_iterable_later;
            std::vector<pair_t> _pairs;
            std::size_t _pair_index;
            std::size_t _pairs_count;
            typename std::vector<pair_t>::iterator _pairs_it;
            // constructor
            inline RangeIterator(btree_t& btree, const key_t& key1, const key_t& key2)
                : _btree(btree)
                , _key1(key1)
                , _key2(key2)
                {}
            inline RangeIterator(btree_t& btree)
                : _btree(btree)
                {}
            // internal method
            inline void _load_values(const page_t& page, page_index_t onpage_index) {
                _pairs.clear();
                do {
                    if (has_start_condition) {
                        if (!start_condition()(page.leaf[onpage_index].key, _key1)) {
                            continue;
                        }
                    }
                    if (must_check_to_continue) {
                        if (!continue_condition()(page.leaf[onpage_index].key, _key2)) {
                            break;
                        }
                    }
                    const auto& pair = page.leaf[onpage_index];
                    _pairs.push_back({pair.key, pair.value});
                } while (++onpage_index < page.keys_count);

                _pairs_it = _pairs.begin();
                _pair_index = 0;
                _pairs_count = _pairs.size();
                _is_iterable = (_pair_index != _pairs_count);
                if (_page_index_path.size() == 0) {
                    _is_iterable_later = false;
                    return;
                }
                _is_iterable_later = true;
                _page_index = *_page_index_path.rbegin();
                _page_index_path.pop_back();
                _onpage_index = *_onpage_index_path.rbegin();
                _onpage_index_path.pop_back();
            }
            // public methods
            inline RangeIterator& begin() {
                _page_index = 0;
                _onpage_index = 0;
                while (_page_index_path.size() < 8) {
                    auto page = _btree._file.page(_page_index);
                    if (page->is_leaf) {
                        // Exceptions::Exception(ERROR, std::to_string(_page_index) + "  " + std::to_string(*page));
                        if (has_start_value) {
                            _onpage_index = _btree.page_leaf_find(*page, _key1);
                        }
                        _load_values(*page, _onpage_index);
                        return *this;
                    } else {
                        // Exceptions::Exception(MESSAGE, std::to_string(_page_index) + "  " + std::to_string(*page));
                        if (has_start_value) {
                            _onpage_index = _btree.page_node_find(*page, _key1);
                        }
                        _onpage_index_path.push_back(_onpage_index);
                        _page_index_path.push_back(_page_index);
                        _page_index = page->node[_onpage_index].page_index;
                    }
                }
                _is_iterable = false;
                return *this;
            }
            inline operator const bool() const {
                return _is_iterable;
            }
            inline void operator++() {
                ++_pairs_it;
                if (++_pair_index == _pairs_count) {
                    if (!_is_iterable_later) {
                        _is_iterable = false;
                        return;
                    }
                    int status = 0;
                    while (_page_index_path.size() < 8) {
                        auto page = _btree._file.page(_page_index);
                        // show1(std::cout, _page_index_path, "[", "] - "); show1(std::cout, _onpage_index_path, "[", "]\n");
                        if (page->is_leaf) {
                            status = 0;
                            // Exceptions::Exception(ERROR, std::to_string(_page_index) + "  " + std::to_string(*page));
                            _load_values(*page, 0);
                            return;
                        }
                        if (++_onpage_index > page->keys_count) {
                            // Exceptions::Exception(WARNING, std::to_string(_page_index) + "  " + std::to_string(*page));
                            if (_page_index_path.size() == 0) {
                                break;
                            }
                            _page_index = *_page_index_path.rbegin();
                            _page_index_path.pop_back();
                            _onpage_index = *_onpage_index_path.rbegin();
                            _onpage_index_path.pop_back();
                            status = 1;
                        } else {
                            // Exceptions::Exception(MESSAGE, std::to_string(_page_index) + "  (" + std::to_string(_onpage_index) + ")  " + std::to_string(*page));
                            switch (status) {
                                case 1: status = 2; break;
                                case 2: _onpage_index = 0; break;
                            }
                            _page_index_path.push_back(_page_index);
                            _page_index = page->node[_onpage_index].page_index;
                            _onpage_index_path.push_back(_onpage_index);
                            _onpage_index = 0;
                        }
                    }
                    _is_iterable = false;
                }
            }
            inline const pair_t& operator*() const {
                return *_pairs_it;
            }
            // conversion
            inline operator Iteration::WrapIterator<std::vector<value_t>> () {
                std::vector<value_t> values;
                for (const value_t& value : *this) {
                    values.push_back(value);
                }
                return Iteration::WrapIterator<std::vector<value_t>>(values);
            }
        };

        typedef RangeIterator<false, false, std::greater_equal<key_t>, false, std::less_equal<key_t>> all_iterator_t;
        inline all_iterator_t all() {
            return all_iterator_t(*this);
        }
        typedef RangeIterator<true, false, std::greater_equal<key_t>, true, std::less_equal<key_t>> isbetween_iterator_t;
        inline isbetween_iterator_t isbetween(const key_t& key1, const key_t& key2) {
            return isbetween_iterator_t(*this, key1, key2);
        }
        typedef RangeIterator<true, false, std::greater_equal<key_t>, false, std::less_equal<key_t>> isgreaterorequal_iterator_t;
        inline isgreaterorequal_iterator_t isgreaterorequal(const key_t& key1) {
            return isgreaterorequal_iterator_t(*this, key1, key1);
        }
        typedef RangeIterator<false, false, std::greater_equal<key_t>, true, std::less_equal<key_t>> issmallerorequal_iterator_t;
        inline issmallerorequal_iterator_t issmallerorequal(const key_t& key1) {
            return issmallerorequal_iterator_t(*this, key1, key1);
        }
        typedef RangeIterator<true, false, std::greater_equal<key_t>, true, std::equal_to<key_t>> isequal_iterator_t;
        inline isequal_iterator_t isequal(const key_t& key1) {
            return isequal_iterator_t(*this, key1, key1);
        }

        inline const std::size_t count(const key_t& key) {
            std::size_t n = 0;
            for (auto it = isequal(key); it; ++it) {
                ++n;
            }
            return n;
        }

        inline const bool page_isfull(const page_t& page) const {
            return (page.is_leaf && page.keys_count >= _leaf_capacity - 1) || page.keys_count >= _node_capacity - 1;
        }

        inline const onpage_index_t page_node_find(const page_t& page, const key_t& key) const {
            if (page.keys_count == 0) {
                return 0;
            }
            for (int i=0; i<page.keys_count; i++) {
                if (key <= page.node[i].key) {
                    return i;
                }
            }
            return page.keys_count;
        }
        inline const onpage_index_t page_leaf_find(const page_t& page, const key_t& key) const {
            if (page.keys_count == 0) {
                return 0;
            }
            for (int i=0; i<page.keys_count; i++) {
                if (key <= page.leaf[i].key) {
                    return i;
                }
            }
            return page.keys_count;
        }

        inline const value_t page_leaf_get(page_t& page, const key_t& key) {
            for (int i=0; i<page.keys_count; i++) {
                if (key == page.leaf[i].key) {
                    return page.leaf[i].value;
                }
            }
            throw Indexing::KeyNotFoundException(_file, key);
        }

        inline void page_node_insertat(page_t& page, const onpage_index_t& onpage_index, const key_t& key, const page_index_t& page_index) {
            // std::cout << "INSERT IN NODE AT " << onpage_index << " / " << page.keys_count << " / " << _node_capacity << std::endl;
            if (onpage_index == page.keys_count) {
                page.node[onpage_index].key = key;
                page.node[onpage_index + 1].page_index = page_index;
                ++page.keys_count;
                return;
            }
            if (onpage_index < page.keys_count) {
                std::size_t n = (std::size_t)page.keys_count - (std::size_t)onpage_index;
                // std::cout << "N = " << n << std::endl;
                memmove(
                    page.node + onpage_index + 1,
                    page.node + onpage_index,
                    n * (sizeof(key_t) + sizeof(page_index_t)) + sizeof(page_index_t)
                );
                ++page.keys_count;
                // std::cout << "MEMMOVE " << (n * (sizeof(onpage_index_t) + sizeof(value_t)) + sizeof(onpage_index_t));
                // std::cout << " FROM " << (char*)(page.node + onpage_index + 1) - (char*)page.node << std::endl;
            }
            page.node[onpage_index + 1].page_index = page_index;
            page.node[onpage_index].key = key;
        }
        inline void page_leaf_insertat(page_t& page, const onpage_index_t& onpage_index, const key_t& key, const value_t& value) {
            // shift to the right, to make some space
            if (onpage_index < page.keys_count++) {
                memmove(
                    page.leaf + onpage_index + 1,
                    page.leaf + onpage_index,
                    (page.keys_count - onpage_index) * (sizeof(key_t) + sizeof(value_t))
                );
            }
            // put the key/value pair at the right place
            page.leaf[onpage_index].key = key;
            page.leaf[onpage_index].value = value;
        }

    };

    template<typename key_t, typename value_t, typename page_index_t, typename onpage_index_t>
    std::ostream& operator << (std::ostream& os, const BTree<key_t, value_t, page_index_t, onpage_index_t>& btree) {
        std::size_t n = btree._file.header()->next_page_index;
        for (std::size_t i=0; i<n; i++) {
            os << i << ": " << * btree._file.page(i) << "\n";
        }
        os << "\n";
        return os;
    }

    #pragma pack(pop)

} // Indexing


#endif // __DUPADB__INDEXING__BTREE_HPP
