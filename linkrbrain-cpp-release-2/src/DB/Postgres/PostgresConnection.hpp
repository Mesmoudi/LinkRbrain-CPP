#ifndef LINKRBRAIN2019__SRC__DB__POSTGRES__POSTGRESCONNECTION_HPP
#define LINKRBRAIN2019__SRC__DB__POSTGRES__POSTGRESCONNECTION_HPP


#include "../Connection.hpp"
#include "./PostgresCursor.hpp"

#include <postgresql/libpq-fe.h>

#include <string>


namespace DB {

    class PostgresConnection : public Connection {
    public:

        PostgresConnection(const std::string& connection_string) :
            _connection_string(connection_string),
            _pg_connection(NULL)
        {
            connect();
        }
        virtual ~PostgresConnection() {
            disconnect();
        }

        virtual Iterator execute_parameters(const std::string& sql, const std::vector<std::string>& parameters) {
            PGresult* result;
            // prepare parameters
            const size_t values_count = parameters.size();
            int* lengths = (int*) malloc(values_count * sizeof(int));
            const char** values = (const char**) malloc(values_count * sizeof(const char*));
            for (size_t i=0, n=parameters.size(); i<n; ++i) {
                values[i] = parameters[i].c_str();
                lengths[i] = parameters[i].size();
            }
            // execute query
            ensure_connection();
            result = PQexecParams(
                _pg_connection,
                sql.c_str(),
                values_count,
                NULL, // Oid field is ignored
                values,
                lengths,
                NULL, // given values are in text format
                0 // returned result is also in text format
            );
            // free parameters
            free(lengths);
            free(values);
            // debugging
            const size_t query_index = compute_new_query_index();
            log_query(query_index, sql, parameters);
            manage_errors(query_index, result);
            // return iterator from cursor
            return {new PostgresCursor(result)};
        }

        virtual Iterator execute(const std::string& sql) {
            PGresult* result;
            // execute query
            ensure_connection();
            result = PQexec(
                _pg_connection,
                sql.c_str()
            );
            // debugging
            const size_t query_index = compute_new_query_index();
            log_query(query_index, sql);
            manage_errors(query_index, result);
            // return iterator from cursor
            return {new PostgresCursor(result)};
        }

        virtual void commit() {
            except("Not implemented");
        }

        virtual const std::string get_engine_name() {
            return "Postgres";
        }

    private:

        void ensure_connection() {
            if (PQstatus(_pg_connection) != CONNECTION_OK || PQsocket(_pg_connection) < 0) {
                get_logger().warning("Connection lost, attempting to reconnect");
                connect();
            }
        }

        void connect() {
            disconnect();
            _pg_connection = PQconnectdb(_connection_string.c_str());
            if (PQstatus(_pg_connection) != CONNECTION_OK) {
                throw Exceptions::DatabaseException("Connection to database failed:", {
                    {"message", PQerrorMessage(_pg_connection)}
                });
            }
            get_logger().message("Successfully connected to Postgres database");
        }
        void disconnect() {
            if (_pg_connection != NULL) {
                PQfinish(_pg_connection);
                get_logger().message("Disconnected from Postgres database");
            }
        }

        inline void manage_errors(const size_t query_index, PGresult* result) {
            switch (PQresultStatus(result)) {
                case PGRES_EMPTY_QUERY:
                    get_logger().warning("Executed query", query_index, ": empty");
                    break;
                case PGRES_COMMAND_OK:
                    get_logger().debug("Executed query", query_index, ": command succeeded");
                    break;
                case PGRES_TUPLES_OK:
                    get_logger().debug("Executed query", query_index, ": fetching tuples succeeded");
                    break;
                case PGRES_SINGLE_TUPLE:
                    get_logger().debug("Executed query", query_index, ": fetching tuple succeeded");
                    break;
                case PGRES_COPY_OUT:
                    get_logger().debug("Executed query", query_index, ": copying from server succeeded");
                    break;
                case PGRES_COPY_BOTH:
                    get_logger().debug("Executed query", query_index, ": copying from/to server succeeded");
                    break;
                case PGRES_COPY_IN:
                    get_logger().debug("Executed query", query_index, ": copying to server succeeded");
                    break;
                case PGRES_BAD_RESPONSE:
                    get_logger().error("Executed query", query_index, ": bad response from server");
                    break;
                case PGRES_NONFATAL_ERROR:
                    get_logger().warning("Executed query", query_index, ": server said:", PQerrorMessage(_pg_connection));
                    break;
                case PGRES_FATAL_ERROR:
                    get_logger().error("Executed query", query_index, ": server said:", PQerrorMessage(_pg_connection));
                    throw Exceptions::DatabaseException("Database error", {
                        {"error_message", PQerrorMessage(_pg_connection)},
                        {"engine", "Postgres"}
                    });
            }
        }

        const std::string _connection_string;
        PGconn *_pg_connection;

    };

} // DB


#endif // LINKRBRAIN2019__SRC__DB__POSTGRES__POSTGRESCONNECTION_HPP
