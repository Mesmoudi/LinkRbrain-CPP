#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__APPCONTROLLER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__APPCONTROLLER_HPP


namespace LinkRbrain::Controllers {
    template <typename T>
    class AppController;
} // LinkRbrain::Controllers


#include "./DataController.hpp"
#include "./DBController.hpp"
#include "./TokensController.hpp"
#include "./HTTPController.hpp"
#include "../Socket/Server.hpp"

#include <filesystem>


namespace LinkRbrain::Controllers {

    template <typename T>
    class AppController : public Logging::Loggable {
    public:

        AppController(
            const std::filesystem::path& socket_path,
            const std::filesystem::path& data_path,
            const DB::Type& db_type,
            const std::string& db_connection_string
        ) :
            _status(Stopped),
            _socket_path(socket_path),
            _data_path(data_path),
            _db_type(db_type),
            _db_connection_string(db_connection_string)
        {
            _socket.reset(new LinkRbrain::Socket::Server<T>(_socket_path, *this));
            _socket->start();
        }
        ~AppController() {}

        enum Status {
            Stopped,
            Starting,
            StartingFailed,
            Started,
            Stopping,
            StoppingFailed,
        };

        static const std::string get_status_name(const Status status) {
            switch (status) {
                case Stopped:
                    return "Stopped";
                case Starting:
                    return "Starting";
                case StartingFailed:
                    return "StartingFailed";
                case Started:
                    return "Started";
                case Stopping:
                    return "Stopping";
                case StoppingFailed:
                    return "StoppingFailed";
            }
            return "(unknown)";
        }

        const std::string get_status_name() const {
            return get_status_name(_status);
        }

        DataController<T>& get_data_controller() {
            if (!_data) {
                throw Exceptions::Exception("Data controller is unavailable");
            }
            return *_data;
        }
        DBController& get_db_controller() {
            if (!_db) {
                throw Exceptions::Exception("Database controller is unavailable");
            }
            return *_db;
        }
        TokensController& get_tokens_controller() {
            if (!_tokens) {
                throw Exceptions::Exception("Tokens controller is unavailable");
            }
            return *_tokens;
        }
        HTTPController<T>& get_http_controller() {
            if (!_http) {
                throw Exceptions::Exception("HTTP controller is unavailable");
            }
            return *_http;
        }

        void start(const bool with_socket=false) {
            if (_status != Stopped) {
                throw Exceptions::Exception("AppController cannot be started from current status '" + get_status_name(_status) + "'");
            }
            _status = Starting;
            try {
                _data.reset(new DataController<T>(_data_path));
                _db.reset(new DBController(_db_type, _db_connection_string));
                _tokens.reset(new TokensController());
                _http.reset(new HTTPController<T>(*this));
                if (with_socket) {
                    _socket.reset(new LinkRbrain::Socket::Server<T>(_socket_path, *this));
                    _socket->start();
                }
                _status = Started;
            } catch (std::exception& exception) {
                _status = StartingFailed;
                throw exception;
            }
        }
        void stop(const bool with_socket=false) {
            if (_status != Started) {
                throw Exceptions::Exception("AppController cannot be stopped from current status '" + get_status_name() + "'");
            }
            _status = Stopping;
            try {
                _http.reset();
                _tokens.reset();
                _db.reset();
                _data.reset();
                if (with_socket) {
                    _socket.reset();
                }
                _status = Stopped;
            } catch (std::exception& exception) {
                _status = StoppingFailed;
                throw exception;
            }
        }

        void restart(const bool with_socket=false) {
            stop(with_socket);
            start(with_socket);
        }

    protected:

        virtual const std::string get_logger_name() {
            return "LinkRbrain::AppController";
        }

    private:

        // Configuration
        const std::filesystem::path _socket_path;
        const std::filesystem::path _data_path;
        const DB::Type _db_type;
        const std::string _db_connection_string;
        // Components
        std::unique_ptr<DataController<T>> _data;
        std::unique_ptr<DBController> _db;
        std::unique_ptr<TokensController> _tokens;
        std::unique_ptr<HTTPController<T>> _http;
        std::unique_ptr<LinkRbrain::Socket::Server<T>> _socket;
        // Others
        Status _status;

    };

} // LinkRbrain::Controllers


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__APPCONTROLLER_HPP
