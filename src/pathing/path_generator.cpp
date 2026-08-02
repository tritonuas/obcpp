#include "pathing/path_generator.hpp"

#include <cmath>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/rrt.hpp"
#include "utilities/datatypes.hpp"

std::vector<std::vector<double>> withStartAngle(std::vector<std::vector<double>> goal_angles,
                                                double start_angle) {
    if (!goal_angles.empty()) {
        goal_angles[0] = {start_angle};
    }

    return goal_angles;
}

PathGenerator::PathGenerator(std::vector<XYZCoord> goals, double start_angle,
                             std::vector<std::vector<double>> goal_angles)
    : rrt(RRTPoint(goals[0], start_angle)),
      goals(std::move(goals)),
      goal_angles(withStartAngle(std::move(goal_angles), start_angle)) {}

PathGenerator::PathGenerator(std::vector<XYZCoord> goals, double start_angle,
                             std::vector<double> angles)
    : PathGenerator(goals, start_angle,
                    std::vector<std::vector<double>>(
                        goals.size(), angles.empty() ? DEFAULT_GOAL_ANGLES : angles)) {}

void PathGenerator::run() {
    generateDubinsOptions();
    generateFlightPoints();
}

void PathGenerator::generateDubinsOptions() {
    const std::size_t total_goals = goals.size();

    // the plane is already sitting on the first goal, it is flown from and never to
    for (std::size_t cur_goal_idx = 1; cur_goal_idx < total_goals; cur_goal_idx++) {
        legs.push_back(rrt.run(goals[cur_goal_idx], goal_angles[cur_goal_idx]));
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

    for (const Leg& leg : legs) {
        const std::vector<XYZCoord> points = buildFlightPath(leg);
        flight_path.insert(flight_path.end(), points.begin(), points.end());
    }
}

std::vector<XYZCoord> PathGenerator::getPointsToGoal() const { return flight_path; }

std::vector<XYZCoord> PathGenerator::buildFlightPath(const Leg& leg) const {
    std::vector<XYZCoord> path = Dubins::generatePath(leg.start, leg.segments);

    if (path.empty()) {
        return path;
    }

    // the leg is flown from the waypoint behind the one it lands on
    const double start_height      = leg.start.coord.z;
    const double height_difference = leg.end.coord.z - start_height;

    // since our points are not evenly spaced, we have to account for distance
    // when doing altitude transitions. This is a misestimate
    XYZCoord previous = leg.start.coord;
    double total_distance = 0;

    for (XYZCoord& point : path) {
        total_distance += std::hypot(point.x - previous.x, point.y - previous.y);
        previous        = point;
        point.z         = total_distance;  // distance flown in path
    }

    // ASSUMPTION: PATH IS NOT A BUNCH OF POINTS ON TOP OF EACH OTHER
    for (XYZCoord& point : path) {
        const double ratio = point.z / total_distance;
        point.z = start_height + height_difference * ratio;
    }

    return path;
}
