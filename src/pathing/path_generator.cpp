#include "pathing/path_generator.hpp"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/rrt.hpp"
#include "utilities/datatypes.hpp"

PathGenerator::PathGenerator(std::vector<XYZCoord> goals, double start_angle,
                             std::vector<std::vector<double>> goal_angles)
    : rrt(RRTPoint(goals[0], start_angle)),
      goals(std::move(goals)),
      goal_angles(std::move(goal_angles)) {}

PathGenerator::PathGenerator(std::vector<XYZCoord> goals, double start_angle,
                             std::vector<double> angles)
    : PathGenerator(goals, start_angle,
                    std::vector<std::vector<double>>(
                        goals.size(), angles)) {}

PathGenerator::PathGenerator(std::vector<XYZCoord> goals, double start_angle)
    : PathGenerator(goals, start_angle,
                    std::vector<std::vector<double>>(
                        goals.size(), DEFAULT_GOAL_ANGLES)) {}

void PathGenerator::run() {
    generateDubinsOptions();
    generateFlightPoints();
}

void PathGenerator::generateDubinsOptions() {
    const std::size_t total_goals = goals.size();

    // the plane is already sitting on the first goal, it is flown from and never to
    for (std::size_t cur_goal_idx = 1; cur_goal_idx < total_goals; cur_goal_idx++) {
        const Leg leg = rrt.run(goals[cur_goal_idx], goal_angles[cur_goal_idx]);
        legs.push_back(leg);
        rrt.reroot(leg.end);  // effectively new RRT instance
    }
}

double PathGenerator::pathLength() const {
    double length = 0;

    for (const Leg& leg : legs) {
        length += leg.length;
    }

    return length;
}

void PathGenerator::generateFlightPoints() {
    flight_path.clear();

    // legs[i] is flown from goals[i] to goals[i + 1]
    for (std::size_t i = 0; i < legs.size(); i++) {
        const std::vector<XYZCoord> points = buildFlightPath(legs[i], goals[i].z, goals[i + 1].z);
        flight_path.insert(flight_path.end(), points.begin(), points.end());
    }
}

std::vector<XYZCoord> PathGenerator::getPointsToGoal() const { return flight_path; }

std::vector<XYZCoord> PathGenerator::buildFlightPath(const Leg& leg, double start_height,
                                                     double end_height) const {
    std::vector<XYZCoord> path = Dubins::generatePath(leg.start, leg.segments);

    const double height_difference = end_height - start_height;

    // since our points are not evenly spaced, we have to account for distance
    // when doing altitude transitions. This is a misestimate
    XYZCoord previous = leg.start.coord;
    double total_distance = 0;

    for (XYZCoord& point : path) {
        total_distance += std::hypot(point.x - previous.x, point.y - previous.y);
        previous        = point;
        point.z         = total_distance;  // placeholder for distance flown in path
    }

    // ASSUMPTION: PATH IS NOT A BUNCH OF POINTS ON TOP OF EACH OTHER
    for (XYZCoord& point : path) {
        const double ratio = point.z / total_distance;
        point.z = start_height + height_difference * ratio;
    }

    return path;
}
