#ifndef LINKRBRAIN2019LINKRBRAIN2019__SRC__DB__ENGINES__POSTGRESCURSOR_HPP
#define LINKRBRAIN2019LINKRBRAIN2019__SRC__DB__ENGINES__POSTGRESCURSOR_HPP


#include "../Cursor.hpp"

#include <postgresql/libpq-fe.h>


namespace DB {

    class PostgresCursor : public Cursor {
    public:

        PostgresCursor(PGresult* pg_result) :
            _pg_result(pg_result) {}
        ~PostgresCursor() {
            PQclear(_pg_result);
        }

        virtual const size_t init() {
            _row_count = PQntuples(_pg_result);
            if (_row_count == 0) {
                return -1;
            }
            _row_size = PQnfields(_pg_result);
            // _pg_types.resize(_row_size);
            // for (size_t i = 0; i < _row_size; ++i) {
            //     _pg_types[i] = PQftype(_pg_result, i);
            // }
            _row_index = 0;
            return _row_size;
        }

        virtual const bool next() {
            return (++_row_index < _row_count);
        }

        virtual const bool get_isnull(const size_t column_index) {
            return PQgetisnull(_pg_result, _row_index, column_index);
        }
        virtual const std::vector<std::string>& get_text() {
            _result.resize(_row_size);
            for (size_t i=0; i<_row_size; ++i) {
                _result[i].assign(
                    PQgetvalue(_pg_result, _row_index, i),
                    PQgetlength(_pg_result, _row_index, i)
                );
            }
            return _result;
        }
        virtual const std::string get_text(const size_t column_index) {
            return {
                PQgetvalue(_pg_result, _row_index, column_index),
                PQgetlength(_pg_result, _row_index, column_index)
            };
        }
        virtual const std::string get_text(const std::string& column_name) {
            for (size_t i=0; i<_row_size; ++i) {
                if (PQfname(_pg_result, i) == column_name) {
                    return get_text(i);
                }
            }
            except("no such column:", column_name);
        }

    private:

        PGresult* _pg_result;
        // std::vector<Oid> _pg_types;
        size_t _row_count;
        size_t _row_index;
        std::vector<std::string> _result;

    };

} // DB


#endif // LINKRBRAIN2019LINKRBRAIN2019__SRC__DB__ENGINES__POSTGRESCURSOR_HPP
