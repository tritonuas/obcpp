#include "pathing/static.hpp"

#include <chrono>
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
    int num_iterations = 100;
    double goal_bias = 0.6;
    double search_radius = 5000;
    double tolerance_to_goal = 50;
    double rewire_radius = 40;
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

    // print out stats
    std::cout << "num_iterations: " << num_iterations << std::endl;
    std::cout << "goal_bias: " << goal_bias << std::endl;
    std::cout << "search_radius: " << search_radius << std::endl;
    std::cout << "tolerance_to_goal: " << tolerance_to_goal << std::endl;
    std::cout << "rewire_radius: " << rewire_radius << std::endl;

    std::cout << "Start Running" << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    rrt.run();

    // time the following function
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << "Time to run: " << elapsed.count() << "s" << std::endl;
    std::cout << "End Running" << std::endl;

    std::vector<XYZCoord> path = rrt.getPointsGoal(true);
    std::cout << path.size() << std::endl;
    std::cout << rrt.getPointsGoal(false).size() << std::endl;


    for (const XYZCoord& point : path) {
        file << point.x << "," << point.y << std::endl;
    }

    file.close();
    return 0;
}