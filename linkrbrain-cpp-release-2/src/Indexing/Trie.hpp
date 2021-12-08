#ifndef LINKRBRAIN2019__SRC__INDEXING__TRIE_HPP
#define LINKRBRAIN2019__SRC__INDEXING__TRIE_HPP


#include "Paged/Directory.hpp"
#include "./Index.hpp"


namespace Indexing {

    #pragma pack(push, 1)

    template<typename key_t, typename keysize_t, typename value_t, typename pageindex_t, typename size_t, keysize_t prefix_limit, typename index_t=char[0]>
    struct TrieHeader : Paged::Header {
        std::size_t next_page_index;
        std::size_t next_index;
    };

    template<typename key_t, typename keysize_t, typename value_t, typename pageindex_t, typename size_t, keysize_t prefix_limit, typename index_t=char[0]>
    struct TriePage {

        struct item_t {
            bool is_leaf : 1;
            keysize_t size : (8 * sizeof(keysize_t) - 1);
            union {
                // leaf
                struct {
                    value_t value;
                    union {
                        struct {
                            char _filler[sizeof(index_t)];
                            char value_key[0];
                        };
                        std::size_t index : sizeof(index_t) ? (8 * sizeof(index_t)) : 8;
                    };
                };
                // node
                struct {
                    pageindex_t pageindex;
                    char pageindex_key[0];
                };
            };
            inline item_t* next() const {
                return (item_t*) ((is_leaf ? value_key : pageindex_key) + size);
            }
        };

        size_t end_position;
        union {
            char data[0];
            item_t first_item[0];
        };

        inline void clear() {
            end_position = 0;
        }
        inline item_t* find(const char* key, const keysize_t size) {
            item_t* item = first_item;
            const item_t* item_end = (item_t*) (data + end_position);
            while (item < item_end) {
                if (item->is_leaf) {
                    if (item->size == size && memcmp(item->value_key, key, item->size) == 0) {
                        return item;
                    }
                } else {
                    if (item->size <= size && memcmp(item->pageindex_key, key, item->size) == 0) {
                        return item;
                    }
                }
                item = item->next();
            }
            return NULL;
        }

        template<typename value__pageindex_t>
        inline item_t* append(const bool is_leaf, const char* key, const keysize_t size, const value__pageindex_t value, const size_t page_size=0) {
            // full size used by the item, including key data
            const keysize_t full_size = sizeof(keysize_t) + (is_leaf
                ? (sizeof(value_t) + sizeof(index_t))
                : sizeof(pageindex_t)
            ) + size;
            // check if we still have space on the current page
            if (page_size != 0 && end_position + full_size > page_size - sizeof(size_t)) {
                return NULL;
            }
            // set values for new item
            item_t* item = (item_t*) (data + end_position);
            item->is_leaf = is_leaf;
            item->size = size;
            if (is_leaf) {
                item->value = value;
                memcpy(item->value_key, key, size);
            } else {
                item->pageindex = value;
                memcpy(item->pageindex_key, key, size);
            }
            // increment end position
            end_position += full_size;
            return item;
        }

        inline const std::string best_prefix() {
            // store prefixes and their respective score
            std::map<std::string, float> prefix_score;
            item_t* item = first_item;
            item_t* item_end = (item_t*) (data + end_position);
            while (item < item_end) {
                keysize_t size_max = std::min(item->size, prefix_limit);
                for (int size=1; size<=size_max; ++size) {
                    const char* item_key = item->is_leaf ? item->value_key : item->pageindex_key;
                    prefix_score[std::string(item_key, size)] += size;
                    // prefix_score[std::string(item->key, size)]++;
                }
                item = item->next();
            }
            // returns the prefix which scores the highest
            std::string best_prefix;
            float best_score = 0.f;
            for (auto it=prefix_score.begin(); it!=prefix_score.end(); ++it) {
                const auto& prefix = it->first;
                const auto& score = it->second;
                if (score > best_score && score > prefix.size()) {
                    best_prefix = prefix;
                    best_score = score;
                }
            }
            return best_prefix;
        }

    };

    #pragma pack(pop)

    template <typename key_t, typename keysize_t, typename value_t, typename pageindex_t, typename size_t, keysize_t prefix_limit, typename index_t=char[0]>
    inline std::ostream& operator<< (std::ostream& os, const TriePage<key_t, keysize_t, value_t, pageindex_t, size_t, prefix_limit, index_t>& page) {
        auto item = page.first_item;
        auto item_end = (typename TriePage<key_t, keysize_t, value_t, pageindex_t, size_t, prefix_limit, index_t>::item_t*) (page.data + page.end_position);
        os << (page.end_position + 0) << " bytes\n";
        while (item < item_end) {
            std::size_t size = sizeof(keysize_t) + item->size;
            if (item->is_leaf) {
                size += sizeof(value_t) + sizeof(index_t);
                os << "L: " << size << ": `";
                os << (item->size ? std::string(item->value_key, item->size) : "") << "`";
                if (sizeof(index_t)) {
                    os << '#' << item->index;
                }
                os << ", " << item->value << "\n";
            } else {
                size += sizeof(pageindex_t);
                os << "N: " << size << ": `";
                os << (item->size ? std::string(item->pageindex_key, item->size) : "") << "`";
                os << ", " << item->pageindex << "\n";
            }
            item = item->next();
        }
        return os;
    }


    template<typename key_t, typename keysize_t, typename value_t, typename pageindex_t, typename size_t, keysize_t prefix_limit, typename index_t=char[0]>
    struct Trie : public Index<key_t, value_t> {

        typedef TrieHeader<key_t, keysize_t, value_t, pageindex_t, size_t, prefix_limit, index_t> header_t;
        typedef TriePage<key_t, keysize_t, value_t, pageindex_t, size_t, prefix_limit, index_t> page_t;
        typedef typename page_t::item_t item_t;


        Paged::File<header_t, page_t>& _file;
        size_t _page_size;

        inline Trie(Paged::Directory& directory, const std::string path, const std::size_t page_size=0)
            : _file(directory.file<header_t, page_t>(path, "TRIE", page_size))
            , _page_size(_file._page_size)
        {}

        inline void split(page_t& page) {
            // retrieve which prefix to factorize
            const std::string prefix = page.best_prefix();
            const char* prefix_data = prefix.data();
            const keysize_t prefix_size = prefix.size();
            // create a new page for this prefix
            const pageindex_t new_page_index = ++_file.header()->next_page_index;
            if (new_page_index == 0) {
                except("Limit of integer reached in trie `%s` (uint%zu_t)", _file._file_path.data(), 8 * sizeof(pageindex_t));
            }
            auto& new_page = *_file.page(new_page_index);
            // loop over the items in the page
            item_t* item = page.first_item;
            const item_t* item_end = (item_t*) (page.data + page.end_position);
            std::vector<std::pair<std::string, item_t>> kept_items;
            while (item < item_end) {
                const char* item_key = item->is_leaf ? item->value_key : item->pageindex_key;
                if (item->size >= prefix.size() && memcmp(item_key, prefix_data, prefix_size) == 0) {
                    if (item->is_leaf) {
                        new_page.append(
                            true,
                            item_key + prefix_size,
                            item->size - prefix_size,
                            item->value
                        );
                    } else {
                        new_page.append(
                            false,
                            item_key + prefix_size,
                            item->size - prefix_size,
                            item->pageindex
                        );
                    }
                } else {
                    kept_items.push_back(std::pair<std::string, item_t>(
                        item->size ? std::string(item_key, item->size) : "",
                        *item
                    ));
                }
                item = item->next();
            }
            // resinsert items to keep
            page.clear();
            page.append(
                false,
                prefix.data(),
                prefix.size(),
                new_page_index
            );
            for (auto it=kept_items.begin(); it!=kept_items.end(); ++it) {
                page.append(
                    it->second.is_leaf,
                    it->first.data(),
                    it->first.size(),
                    it->second.value
                );
            }
            // std::cout << "\nOLD PAGE: " << page << "\n\n" << "CREATED NEW PAGE #" << new_page_index << ": " << new_page << "\n\n";
        }

        inline void insert(const std::string& key, const value_t& value) {
            insert(key.data(), key.size(), value);
        }
        inline void insert(const char* key, const value_t& value) {
            insert(key, strlen(key), value);
        }
        inline void insert(const char* key, const keysize_t size, const value_t& value) {
            pageindex_t page_index = 0;
            const char* current_key = key;
            keysize_t current_size = size;
            while (true) {
                auto& page = * _file.page(page_index);
                item_t* item = page.find(current_key, current_size);
                if (item == NULL) {
                    // no match has been found on this page
                    item = page.append(true, current_key, current_size, value, _page_size);
                    if (item) {
                        if (sizeof(index_t) > 0) {
                            // the new item has been properly inserted in the page, exit
                            item->index = _file.header()->next_index++;
                        }
                        return;
                    }
                    // split the page, try to append again, exit
                    split(page);
                    continue;
                } else {
                    // a match has been found on this page
                    if (item->is_leaf) {
                        // update value and exit
                        item->value = value;
                        return;
                    } else {
                        // update pointers
                        if (current_size < item->size) {
                            except("Cannot decrease size during insertion in Trie `" + _file._file_path + "`");
                        }
                        page_index = item->pageindex;
                        current_key += item->size;
                        current_size -= item->size;
                    }
                }
            }
        }
        virtual void insert(const key_t& key, const value_t& value) {
            insert((const char*)&key, sizeof(key), value);
        }

        inline value_t& get_reference(const std::string& key, const value_t& default_value) {
            return get_reference(key.data(), key.size(), default_value);
        }
        inline value_t& get_reference(const char* key, const keysize_t size, const value_t& default_value) {
            pageindex_t page_index = 0;
            const char* current_key = key;
            keysize_t current_size = size;
            while (true) {
                auto& page = * _file.page(page_index);
                item_t* item = page.find(current_key, current_size);
                if (item == NULL) {
                    // no match has been found on this page
                    item = page.append(true, current_key, current_size, default_value, _page_size);
                    if (item) {
                        if (sizeof(index_t) > 0) {
                            // the new item has been properly inserted in the page, exit
                            item->index = _file.header()->next_index++;
                        }
                        return item->value;
                    }
                    // split the page, try to append again, exit
                    split(page);
                    continue;
                } else {
                    // a match has been found on this page
                    if (item->is_leaf) {
                        // update value and exit
                        return item->value;
                    } else {
                        // update pointers
                        if (current_size < item->size) {
                            except("Cannot decrease size during insertion in Trie `" + _file._file_path + "`");
                        }
                        page_index = item->pageindex;
                        current_key += item->size;
                        current_size -= item->size;
                    }
                }
            }
        }

        inline const value_t get(const std::string& key) {
            return get(key.data(), key.size());
        }
        inline const value_t get(const char* key, const keysize_t size) {
            pageindex_t page_index = 0;
            const char* current_key = key;
            keysize_t current_size = size;
            while (true) {
                page_t& page = * _file.page(page_index);
                item_t* item = page.find(current_key, current_size);
                if (item == NULL) {
                    throw Indexing::KeyNotFoundException(key);
                } else {
                    // a match has been found on this page
                    if (item->is_leaf) {
                        // return value
                        return item->value;
                    } else {
                        // update pointers
                        if (current_size < item->size) {
                            except("Cannot decrease size during retrieval in Trie `" + _file._file_path + "`");
                        }
                        page_index = item->pageindex;
                        current_key += item->size;
                        current_size -= item->size;
                    }
                }
            }
        }
        virtual const value_t get(const key_t& key) {
            return get((const char*)&key, sizeof(key_t));
        }

    };

    template<typename key_t, typename keysize_t, typename value_t, typename pageindex_t, typename size_t, keysize_t prefix_limit, typename index_t=char[0]>
    inline std::ostream& operator<< (std::ostream& os, Trie<key_t, keysize_t, value_t, pageindex_t, size_t, prefix_limit, index_t>& trie) {
        pageindex_t last_page_index = trie._file.header()->next_page_index;
        for (pageindex_t page_index=0; page_index<last_page_index; ++page_index) {
            std::cout << "PAGE #" << page_index << ": " << * trie._file.page(page_index) << std::endl;
        }
        return os;
    }

} // Indexing



#endif // LINKRBRAIN2019__SRC__INDEXING__TRIE_HPP
