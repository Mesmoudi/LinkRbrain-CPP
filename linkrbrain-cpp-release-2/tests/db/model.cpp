#include "ORM/SimpleModel.hpp"

#include "Exceptions/Exception.hpp"
#include "Logging/Loggers.hpp"
auto& logger = Logging::get_logger();


class Queries : public ORM::SimpleModel {
protected:

    virtual std::string get_table_name() const {
        return "queries";
    }
};


int main(int argc, char const *argv[]) {

    logger.debug("The end!");
    return 0;
}
