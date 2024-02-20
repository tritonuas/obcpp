#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>

#include "pathing/static.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

int main() {
    std::cout << "Messing with RRT*" << std::endl;
    std::ofstream file;
    file.open("path_coordinates.txt");

    // RRTPoint test;
    RRTPoint start{XYZCoord(100, 100, 0), M_PI / 6.0};
    std::vector<XYZCoord> goals{XYZCoord{400, 150, 0}, XYZCoord{400, 500, 0},
                                XYZCoord{150, 700, 0}};
    int num_iterations = 90;
    double search_radius = 200;
    double rewire_radius = 40;
    Polygon bounds;
    bounds.emplace_back(XYZCoord{0, 0, 0});
    bounds.emplace_back(XYZCoord{500, 0, 0});
    bounds.emplace_back(XYZCoord{500, 800, 0});
    bounds.emplace_back(XYZCoord{0, 800, 0});
    bounds.emplace_back(XYZCoord{0, 600, 0});
    bounds.emplace_back(XYZCoord{300, 600, 0});
    bounds.emplace_back(XYZCoord{300, 300, 0});
    bounds.emplace_back(XYZCoord{0, 300, 0});

    RRT rrt = RRT(start, goals, num_iterations, search_radius, rewire_radius, bounds);

    // print out stats
    std::cout << "num_iterations: " << num_iterations << std::endl;
    std::cout << "search_radius: " << search_radius << std::endl;
    std::cout << "rewire_radius: " << rewire_radius << std::endl;

    std::cout << "Start Running" << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    rrt.run();

    // time the following function
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << "Time to run: " << elapsed.count() << "s" << std::endl;
    std::cout << "End Running" << std::endl;

    // std::vector<XYZCoord> path = rrt.getPointsGoal(true);
    // std::cout << path.size() << std::endl;
    // std::cout << rrt.getPointsGoal(false).size() << std::endl;

    // for (const XYZCoord& point : path) {
    //     file << point.x << "," << point.y << std::endl;
    // }

    file.close();
    return 0;
}