#include "Logging/Loggers.hpp"

#include <vector>
#include <thread>


static const int threads_count = 8;


void log_number(const int i) {
    Logging::get_logger("MULTITHREAD").message("[", i, "]");
}


int main(int argc, char const *argv[]) {

    Logging::add_output(Logging::Output::StandardError).set_color(true);
    Logging::add_output("var/log").set_color(false);

    // // test direct Logger instantiation
    // Logging::Logger logger(std::cerr, true);
    Logging::Logger& logger = Logging::get_logger("haha");
    logger.warning();
    logger.message(12, "dhfg", 741.5);
    logger.error(12, "dhfg", 741.5);
    logger.notice(12, "dhfg", 741.5);
    logger.debug(12, "dhfg", 741.5);
    logger.fatal(12, "dhfg", 741.5);

    // test get_logger
    Logging::Logger& test_logger = Logging::get_logger("TEST");
    test_logger.notice(":)");
    test_logger.fatal(":(");
    Logging::get_logger("TEST").debug("OK");

    // test multithreading
    std::thread threads[threads_count];
    for (int i=0; i<threads_count; ++i) {
        threads[i] = std::thread(log_number, i + 1);
    }
    for (auto& th : threads) th.join();

    return 0;
}
