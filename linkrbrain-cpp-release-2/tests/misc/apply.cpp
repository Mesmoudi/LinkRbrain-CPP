#include "Exceptions/Exception.hpp"
#include "Logging/Loggers.hpp"
auto& logger = Logging::get_logger();


static const size_t N = 1 << 24;
static const size_t P = 1 << 4;


#include <utility>
#include <vector>


int sum(size_t p, std::vector<int>& a, std::vector<int>& b) {
    int s = 0;
    for (size_t i=0; i<p; ++i) {
        s += a[i] + b[i];
    }
    return s;
}


int main(int argc, char const *argv[]) {

    std::vector<int> a, b;
    std::tuple<std::vector<int>&, std::vector<int>&> vectors(a, b);
    for (size_t i=0; i<P; ++i) {
        a.push_back(i);
        b.push_back(i);
    }
    logger.debug("Generated values");

    for (int i=0; i<2; ++i) {

        int s = 0;
        for (size_t i=0; i<N; ++i) {
            s += sum(P, a, b);
        }
        logger.notice("Called function normally, sum is", s);

        s = 0;
        for (size_t i=0; i<N; ++i) {
            s += std::apply(sum,
                std::tuple_cat(std::tuple<size_t>(P), vectors));
                // std::tuple<size_t, std::vector<int>&, std::vector<int>&>(P, a, b));
        }
        logger.message("Called function with std::apply, sum is", s);

    }

    return 0;
}
