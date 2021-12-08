#include "DB/Database.hpp"


static const std::string connection_string = "host=localhost user=linkrbrain_user password=linkrbrain_password dbname=linkrbrain2019";


int main(int argc, char const *argv[]) {
    DB::Database db(DB::Postgres, connection_string);
    DB::Connection& connection = db.get_connection();
    auto result = connection.execute("SELECT * FROM queries");
    for (auto& item : result) {
        std::cout << item.get_text(0) << '\n';
        std::cout << item.get_text(1) << '\n';
        std::cout << item.get_text(2) << '\n';
        std::cout << item.get_text("id") << '\n';
        std::cout << item.get_text("label") << '\n';
    }
    return 0;
}
