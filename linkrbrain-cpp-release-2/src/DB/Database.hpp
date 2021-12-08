#ifndef LINKRBRAIN2019__SRC__DB__DATABASE_HPP
#define LINKRBRAIN2019__SRC__DB__DATABASE_HPP


#include "./Type.hpp"
#include "./Connection.hpp"
#include "./connect.hpp"

#include "Exceptions/Exception.hpp"
#include "Logging/Loggable.hpp"

#include <functional>


namespace DB {

    class Database : public Logging::Loggable {
    public:

        template <typename ... ParametersTypes>
        Database(const Type& type, const ParametersTypes& ... parameters) : _type(type) {
            // _make_connection = std::bind<const Type&, const ParametersTypes& ...>(connect, type, parameters ...);
            // _make_connection = std::bind<Connection*(const Type&, const ParametersTypes& ...), Connection*>(connect, type, parameters ...);
            _make_connection = std::bind(connect<ParametersTypes ...>, type, parameters ...);
        }

        Type& get_type() {
            return _type;
        }

        const size_t& get_pool_size() const {
            return _pool_size;
        }
        void set_pool_size(const size_t pool_size) {
            _pool_size = pool_size;
        }

        Connection& get_connection() {
            if (_pool.size() == 0) {
                _pool.push_back(
                    std::shared_ptr<Connection>(
                        _make_connection()
                    )
                );
            }
            return * _pool[0];
        }

    protected:

        virtual const std::string get_logger_name() {
            return "DB::Database[" + get_connection().get_engine_name() + "]";
        }

    private:

        Type _type;
        std::function<Connection*()> _make_connection;
        size_t _pool_size;
        std::vector<std::shared_ptr<Connection>> _pool;

    };

} // DB


#endif // LINKRBRAIN2019__SRC__DB__DATABASE_HPP
