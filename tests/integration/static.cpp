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
    RRTPoint start{XYZCoord(100, 100, 0), M_PI / 6.0};
    std::vector<RRTPoint> goals{RRTPoint(XYZCoord{400, 150, 0}, M_PI / 4),
                                RRTPoint(XYZCoord{400, 500, 0}, HALF_PI),
                                RRTPoint(XYZCoord{150, 700, 0}, M_PI)};
    int num_iterations = 999999999;
    double goal_bias = 0.1;
    double search_radius = 45;
    double tolerance_to_goal = 5;
    double rewire_radius = 500;
    Polygon bounds;
    bounds.emplace_back(XYZCoord{0, 0, 0});
    bounds.emplace_back(XYZCoord{500, 0, 0});
    bounds.emplace_back(XYZCoord{500, 800, 0});
    bounds.emplace_back(XYZCoord{0, 800, 0});
    bounds.emplace_back(XYZCoord{0, 600, 0});
    bounds.emplace_back(XYZCoord{300, 600, 0});
    bounds.emplace_back(XYZCoord{300, 300, 0});
    bounds.emplace_back(XYZCoord{100, 300, 0});

    RRT rrt = RRT(start, goals, num_iterations, goal_bias, search_radius, tolerance_to_goal,
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