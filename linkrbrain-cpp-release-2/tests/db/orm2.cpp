#include "DB/ORM/Controller.hpp"
#include "LinkRbrain/Models/Query.hpp"
auto& logger = Logging::get_logger();


typedef LinkRbrain::Models::Query Query;
typedef DB::ORM::Controller<Query> QueriesController;


static const DB::Type connection_type = DB::Postgres;
static const std::string connection_string = "host=localhost user=linkrbrain_user password=linkrbrain_password dbname=linkrbrain2019";


int main(int argc, char const *argv[]) {

    DB::Database db(connection_type, connection_string);
    QueriesController queries_controller(db);
    Query _ = queries_controller.fetch(1);

    for (auto& query : queries_controller.fetch_all()) {
        std::cout << query.id << ": " << query.name << '\n';
    }
    logger.notice("Iterated over queries");

    Query q = queries_controller.fetch(1);
    logger.notice("Fetched first query");
    std::cout << q.name << '\n';
    std::cout << q.groups << '\n';
    std::cout << q.settings << '\n';
    std::cout << q.permissions << '\n';
    std::cout << q.created_at << '\n';
    logger.notice("Showed first query");

    queries_controller.insert(q);
    logger.notice("Re-inserted first query");
    std::cout << q.id << '\n';
    q.name += " [renamed]";
    queries_controller.update(q, {"name", "groups"});
    logger.notice("Renamed inserted query");
    queries_controller.remove(q);
    logger.notice("Removed inserted query");
    queries_controller.remove(q);

    Query q2 = queries_controller.insert_data({
        {"name", "TEST"},
        {"groups", Types::Variant::Vector()},
        {"settings", {{"view", "3D"}}},
        {"permissions", {{"own", {1}}, {"write", {1}}, {"read", {1}}}}
    });
    logger.notice("Newly inserted query from variant");
    queries_controller.update_data(q2, {
        {"name", "RENAMED"}
    });
    logger.notice("Renamed newly inserted query");
    queries_controller.remove(q2);
    logger.notice("Removed newly inserted query");

    Types::Variant serialized;
    queries_controller.serialize(serialized, q2);
    logger.debug("Serialized deleted query");
    Buffering::Writing::StringWriter serialized_writer(Buffering::JSON);
    serialized_writer << serialized;
    logger.debug(serialized_writer.get_string());
    logger.notice("Serialized deleted query");

    return 0;
}
