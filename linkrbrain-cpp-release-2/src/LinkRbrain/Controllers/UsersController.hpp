#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__USERSCONTROLLER_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__USERSCONTROLLER_HPP


#include "DB/ORM/Controller.hpp"

#include "../Models/User.hpp"


namespace LinkRbrain::Controllers {


    class UsersController : public DB::ORM::Controller<Models::User> {
    public:

        using DB::ORM::Controller<Models::User>::Controller;

        Models::User fetch_by_credentials(const std::string& username, const std::string& password) {
            Models::User user;
            std::string sql;
            for (size_t i=0, n=_fields.size(); i<n ;++i) {
                sql += i ? ", " : "SELECT ";
                sql += _fields[i].name;
            }
            //
            sql += " FROM " + _table_name;
            sql += " WHERE username = $1 AND password = $2";
            DB::Iterator iterator = _database.get_connection().execute(sql, username, password).begin();
            if (iterator.begin() == iterator.end()) {
                throw Exceptions::NotFoundException("Cannot find user with these credentials", {
                    {"problem", "invalidcredentials"},
                    {"resource", "users"}
                });
            }
            set_instance(user, * iterator.get_cursor());
            return user;
        }

    };

} // LinkRbrain::Controllers


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__CONTROLLERS__USERSCONTROLLER_HPP
