#include "LinkRbrain/Models/Point.hpp"
#include "LinkRbrain/Correlation/DistanceCorrelator.hpp"
#include "LinkRbrain/Correlation/SphereCorrelator.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Random.hpp"

#include <iostream>
#include <vector>


const bool use_cache = false;
const size_t n = 1e6;
const size_t m = 5;
const int seed = 1;
const double diameter = 5.;


int main(int argc, char const *argv[]) {
    Logger logger(std::cerr, true);
    Correlation::DistanceCorrelator<use_cache> correlator(diameter);
    // LinkRbrain::Correlation::SphereCorrelator<use_cache> correlator(diameter);
    //
    LinkRbrain::Models::WeightedPoint p1(2, 4, 6);
    LinkRbrain::Models::WeightedPoint p2(2, 4, 15);
    logger.debug(p1);
    logger.debug(p2);
    logger.debug(correlator.score(p1, p2));
    //
    std::vector<LinkRbrain::Models::WeightedPoint> points;
    Utils::Random::reseed(seed);
    for (size_t i=0; i<n; ++i) {
        points.push_back({
            Utils::Random::generate_number(-68, 70),
            Utils::Random::generate_number(-108, 68),
            Utils::Random::generate_number(-70, 78),
            Utils::Random::generate_number(1.0),
        });
    }
    points[0].normalized_weight = points[0].weight = 1.0;
    for (int i=1; i<=m; ++i) {
        const double dz = diameter * (double)i / (double)m;
        points[i] = LinkRbrain::Models::WeightedPoint(points[0].x, points[0].y, points[0].z + dz);
    }
    logger.notice("generated", points.size(), "points");
    for (int i=0; i<=m; ++i) {
        logger.debug(points[i]);
    }
    //
    std::vector<double> scores;
    scores.resize(n);
    logger.notice("resized scores to", scores.size());
    for (size_t i=0; i<n; ++i) {
        scores[i] = correlator.score(points[0], points[i]);
    }
    logger.notice("computed", scores.size(), "scores");
    for (int i=0; i<=m; ++i) {
        logger.debug(scores[i]);
    }
    for (size_t i=0; i<n; ++i) {
        scores[i] = correlator.score(points[0], points[i]);
    }
    logger.notice("recomputed", scores.size(), "scores");
    for (int i=0; i<=m; ++i) {
        logger.debug(scores[i]);
    }
    //
    return 0;
}
