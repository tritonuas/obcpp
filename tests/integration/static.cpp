#include "pathing/static.hpp"

#include <cmath>
#include <iostream>

#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

int main() {
    std::cout << "Messing with RRT*" << std::endl;

    // RRTPoint test;
    RRTPoint start{XYZCoord(5.0, 5.0, 0), M_PI / 6.0};
    RRTPoint goal{XYZCoord{350, 700, 0}, HALF_PI};
    int num_iterations = 100;
    double goal_bias = 0.1;
    double search_radius = 30;
    double tolerance_to_goal = 10;
    double rewire_radius = 30;
    Polygon bounds{FLIGHT_BOUND_COLOR};
    bounds.emplace_back(XYZCoord{0, 0, 0});
    bounds.emplace_back(XYZCoord{500, 0, 0});
    bounds.emplace_back(XYZCoord{500, 800, 0});
    bounds.emplace_back(XYZCoord{0, 800, 0});

    return 0;
}