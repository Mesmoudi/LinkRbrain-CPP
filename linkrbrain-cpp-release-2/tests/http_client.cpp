#include "Exceptions/Exception.hpp"
#include "Buffering/Writing/StringWriter.hpp"
#include "Network/Client/RestClient.hpp"
#include "Logging/Loggers.hpp"
auto& logger = Logging::get_logger();


static const std::string base_url = "https://reqres.in/api";


const std::string JSON_encode(const Types::Variant& source) {
    Buffering::Writing::StringWriter buffer(Buffering::JSON);
    buffer << source;
    return buffer.get_string();
}


int main(int argc, char const *argv[]) {

    // initialization
    Network::Client::RestClient client(base_url, Buffering::JSON);
    size_t id;

    // GET
    {
        const auto response = client.get("/users");
        logger.notice("Retrieved from", response.method, response.url, ": code =", response.code, "/ data =", JSON_encode(response.data));
        logger.debug("data[\"data\"] =", JSON_encode(response.data["data"]));
        logger.debug("data[\"data\"][0] =", JSON_encode(response.data["data"][0]));
        id = response.data["data"][0]["id"];
        logger.debug("data[\"data\"][0][\"id\"] =", id);
        logger.message("Performed GET test");
    }

    // GET
    {
        const auto response = client.get("/users/" + std::to_string(id));
        logger.notice("Retrieved from", response.method, response.url, ": code =", response.code, "/ data =", JSON_encode(response.data));
        logger.message("Performed POST test");
    }

    // POST
    {
        const Types::Variant payload = {{"name","Foo"}, {"job","bar"}};
        logger.debug("Payload:", JSON_encode(payload));
        const auto response = client.post("/users", payload);
        logger.notice("Retrieved from", response.method, response.url, ": code =", response.code, "/ data =", JSON_encode(response.data));
        logger.message("Performed POST test");
    }


    return 0;
}
