#include "Types/Array3D.hpp"
#include "Logging/Loggers.hpp"
auto& logger = Logging::get_logger();


typedef double Floating;
static const Floating resolution = 2.0;
static const Floating diameter = 10.0;
// static const Types::Point<Floating> min = {-60, -90, -70};
// static const Types::Point<Floating> max = {+60, +60, +70};
static const Types::Point<Floating> min = {-3.2, -4.3, -5.8};
static const Types::Point<Floating> max = {+5.9, +4.9, +7.9};


int main(int argc, char const *argv[]) {
    Types::PointExtrema<Floating> extrema(min, max);
    logger.debug("Got extrema:", extrema.min, extrema.max);
    logger.debug("Inflated functions extrema:", extrema.min, extrema.max);

    logger.debug("Box starts at:", extrema.min);
    logger.debug("Box ends at:", extrema.max);
    Types::Array3D<Floating> density(extrema, resolution);
    logger.notice("Instanciated 3D array");
    Floating sum = 0.;
    size_t count = 0;
    for (auto& point : density) {
        sum += *point.value;
        ++count;
        // logger.debug("index =", point.index, "; value =", point.value);
        // logger.debug("index =", point.index, "; coordinates =", point.coordinates);
    }
    logger.debug("Computed sum over", density.get_size(), "(" + std::to_string(count) + ") points:", sum);

    const Types::PointExtrema<double> window(
        Types::Point<double>(-2,-2,-2),
        Types::Point<double>(+2,+2,+2)
    );
    for (auto& point : density.restrict_coordinates(window)) {
        *point.value = 1.0;
        logger.debug("index =", point.index, "; coordinates =", point.coordinates);
    }
    logger.notice("Filled part of the matrix");
    for (auto& point : density) {
        if (* point.value) {
            logger.debug("index =", point.index, "; coordinates =", point.coordinates);
        }
    }
    return 0;
}
