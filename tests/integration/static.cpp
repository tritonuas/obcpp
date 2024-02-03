#include "pathing/static.hpp"

#include <cmath>
#include <fstream>
#include <iostream>

#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

int main() {
    std::cout << "Messing with RRT*" << std::endl;
    std::ofstream file;
    file.open("path_coordinates.txt");

    // RRTPoint test;
    RRTPoint start{XYZCoord(5.0, 5.0, 0), M_PI / 6.0};
    RRTPoint goal{XYZCoord{350, 500, 0}, HALF_PI};
    int num_iterations = 100;
    double goal_bias = 0.01;
    double search_radius = 50;
    double tolerance_to_goal = 30;
    double rewire_radius = 30;
    Polygon bounds{FLIGHT_BOUND_COLOR};
    bounds.emplace_back(XYZCoord{0, 0, 0});
    bounds.emplace_back(XYZCoord{500, 0, 0});
    bounds.emplace_back(XYZCoord{500, 800, 0});
    bounds.emplace_back(XYZCoord{0, 800, 0});

    RRT rrt = RRT(start, goal, num_iterations, goal_bias, search_radius, tolerance_to_goal,
                  rewire_radius, bounds);

    std::cout << "Start Running" << std::endl;
    rrt.run();
    std::cout << "End Running" << std::endl;

    for (const XYZCoord& point : rrt.getPointsGoal()) {
        file << point.x << "," << point.y << std::endl;
    }

    file.close();
    return 0;
}