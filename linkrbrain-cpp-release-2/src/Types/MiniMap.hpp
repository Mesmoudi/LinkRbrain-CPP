#ifndef LINKRBRAIN2019__SRC__TYPES__MINIMAP_HPP
#define LINKRBRAIN2019__SRC__TYPES__MINIMAP_HPP


#include <vector>
#include "Exceptions/GenericExceptions.hpp"


namespace Types {


    template <typename K, typename V>
    class MiniMap {
    public:

        typedef std::pair<K, V> T;

        void insert(const T& key_value) {
            _values.push_back(key_value);
        }
        void insert(const K& key, const V& value) {
            _values.push_back({key, value});
        }
        V& get(const K& key) {
            for (size_t i=0, n=_values.size(); i<n; ++i) {
                T& key_value = _values[i];
                if (key_value.first == key) {
                    return key_value.second;
                }
            }
            throw Exceptions::NotFoundException("Key could not be found in MiniMap", {{"key", key}});
        }
        const V& get(const K& key) const {
            for (size_t i=0, n=_values.size(); i<n; ++i) {
                const T& key_value = _values[i];
                if (key_value.first == key) {
                    return key_value.second;
                }
            }
            throw Exceptions::NotFoundException("Key could not be found: ", {{"key", key}});
        }
        const V& get(const K& key, const V& value) const {
            for (size_t i=0, n=_values.size(); i<n; ++i) {
                const T& key_value = _values[i];
                if (key_value.first == key) {
                    return key_value.second;
                }
            }
            return value;
        }
        const std::vector<V> get_all(const K& key) const {
            std::vector<V> result;
            for (size_t i=0, n=_values.size(); i<n; ++i) {
                const T& key_value = _values[i];
                if (key_value.first == key) {
                    result.push_back(key_value.second);
                }
            }
            return result;
        }
        const std::vector<T>& get_all() const {
            return _values;
        }
        const size_t size() const {
            return _values.size();
        }

    private:

        std::vector<T> _values;

    };


} // Types


#endif // LINKRBRAIN2019__SRC__TYPES__MINIMAP_HPP
