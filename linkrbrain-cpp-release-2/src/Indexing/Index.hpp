#ifndef LINKRBRAIN2019__SRC__INDEXING__INDEX_HPP
#define LINKRBRAIN2019__SRC__INDEXING__INDEX_HPP


#include "./exceptions.hpp"


namespace Indexing {

    template <typename key_t, typename value_t>
    class Index {
    public:

        virtual ~Index() {}

        virtual void insert(const key_t& key, const value_t& value) = 0;

        virtual const value_t get(const key_t& key) = 0;
        inline const value_t get(const key_t& key, const value_t& value) {
            try {
                return get(key);
            } catch (KeyNotFoundException const&) {
                return value;
            }
        }

        const value_t operator[](const key_t& key) {
            return Register(*this, key);
        }

    private:

        class Register {
        public:
            inline Register(Index& index, const key_t& key) : _index(index), _key(key) {}
            inline Register& operator= (const value_t& value) {
                _index.insert(_key, value);
                return *this;
            }
            inline operator const value_t() {
                return _index.get(_key);
            }
        private:
            Index& _index;
            const key_t _key;
        };

    };

}

#endif // LINKRBRAIN2019__SRC__INDEXING__INDEX_HPP
