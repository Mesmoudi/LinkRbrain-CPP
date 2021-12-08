#ifndef LINKRBRAIN2019__SRC__DB__ENGINE_HPP
#define LINKRBRAIN2019__SRC__DB__ENGINE_HPP


#include "./Connection.hpp"

#include "Exceptions/Exception.hpp"
#include "Logging/Loggable.hpp"


namespace DB {


    class Engine : public Logging::Loggable {

        const size_t& get_pool_size() const {
            return _pool_size;
        }
        void set_pool_size(const size_t pool_size) {
            _pool_size = pool_size;
        }

        Connection& get_connection() {
            if (_pool.size() == 0) {
                _pool.push_back(
                    std::make_shared<Connection>(make_connection())
                );
            }
            return * _pool[0];
        }

    protected:

        virtual Connection* make_connection() = 0;

    private:

        size_t _pool_size;
        std::vector<std::shared_ptr<Connection>> _pool;

    };


} // DB


#endif // LINKRBRAIN2019__SRC__DB__ENGINE_HPP
