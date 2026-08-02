#include "pathing/rrt.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "pathing/tree.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/logging.hpp"
#include "utilities/rng.hpp"

std::vector<std::vector<double>> withStartAngle(std::vector<std::vector<double>> goal_angles,
                                                double start_angle) {
    if (!goal_angles.empty()) {
        goal_angles[0] = {start_angle};
    }

    return goal_angles;
}

RRT::RRT(std::vector<XYZCoord> goals, double start_angle,
         std::vector<std::vector<double>> goal_angles)
    : tree(RRTPoint(goals[0], start_angle)),
      goals(std::move(goals)),
      goal_angles(withStartAngle(std::move(goal_angles), start_angle)) {}

RRT::RRT(std::vector<XYZCoord> goals, double start_angle, std::vector<double> angles)
    : RRT(goals, start_angle,
          std::vector<std::vector<double>>(goals.size(),
                                           angles.empty() ? DEFAULT_GOAL_ANGLES : angles)) {}

void RRT::run() {
    generateDubinsOptions();
    generateFlightPoints();
}

void RRT::generateDubinsOptions() {
    const uint8_t total_goals = goals.size();

    for (uint8_t cur_goal_idx = 1; cur_goal_idx < total_goals; cur_goal_idx++) {
        // tries to connect directly to the goal from start
        if (connectToGoal(cur_goal_idx)) {
            continue;
        }

        RRTIteration(cur_goal_idx);
    }
}

double RRT::pathLength() const {
    double length = 0;

    for (const Leg& leg : legs) {
        length += leg.length;
    }

    return length;
}

void RRT::generateFlightPoints() {
    flight_path.clear();

    for (const Leg& leg : legs) {
        const std::vector<XYZCoord> points = buildFlightPath(leg);
        flight_path.insert(flight_path.end(), points.begin(), points.end());
    }
}

std::vector<XYZCoord> RRT::getPointsToGoal() const { return flight_path; }

bool RRT::RRTIteration(uint8_t cur_goal_idx) {
    std::vector<RRTPoint> sample(1);

    for (NodeId _ = 0; _ < ITERATIONS_PER_WAYPOINT; _++) {
        sample[0] = RRTPoint(
						Environment::getRandomPoint(false, goals[cur_goal_idx]),
						random(0, TWO_PI)
					);

        // adds the sample to the tree if there is any way to fly to it
        const Connection connection = bestConnection(sample);

        if (connection.isValid()) {
            tree.addSample(connection.anchor, connection.end, connection.option);
        }
    }

    if (connectToGoal(cur_goal_idx)) {
        return true;
    }

    loguru::set_thread_name("Static Pathing");
    LOG_F(WARNING, "Failed to connect to goal on iteration: [%s]. Trying again...",
          std::to_string(cur_goal_idx).c_str());

    // throws away the tree that failed, keeping the same starting point
    tree.setCurrentHead(tree.getStart());

    // TODO: possiblility for infinite loop
    return RRTIteration(cur_goal_idx);
}

double RRT::lowerBound(NodeId node, const std::vector<RRTPoint>& ends) const {
    const XYZCoord& anchor = tree.tree.points[node].coord;
    double closest 		   = std::numeric_limits<double>::infinity();

    for (const RRTPoint& end : ends) {
        closest = std::min(closest, anchor.distanceTo(end.coord));
    }

    return tree.tree.length[node] + closest;
}

void RRT::fillOptions(NodeId node, const std::vector<RRTPoint>& ends) const {
    options.clear();
    const RRTPoint& anchor = tree.tree.points[node];
    const double flown 	   = tree.tree.length[node];

    for (const RRTPoint& end : ends) {
        // gets all dubins curves from the given node to the end point
        for (const RRTOption& option : Dubins::allOptions(anchor, end)) {
            // filters out the options that are not valid
            if (!std::isfinite(option.length)) {
                continue;
            }

            options.push_back({node, end, option, flown + option.length});
        }
    }
}

Connection RRT::bestConnection(const std::vector<RRTPoint>& ends) const {
    for (NodeId node = 0; node < tree.tree.size; node++) {
        bounds[node]   = lowerBound(node, ends);
        frontier[node] = node;
    }

    NodeId remaining = tree.tree.size;
    const auto cheapest_last = [this](NodeId a, NodeId b) { return bounds[a] > bounds[b]; };
    // sorted since after the first few nodes, the remaining often get skipped
    std::make_heap(frontier.begin(), frontier.begin() + remaining, cheapest_last);

    Connection best;
    while (remaining > 0) {
        std::pop_heap(frontier.begin(), frontier.begin() + remaining, cheapest_last);
        const NodeId node = frontier[--remaining];

        // the rest of the tree is at least this expensive, so it cannot do better
        if (bounds[node] >= best.cost) {
            break;
        }

        fillOptions(node, ends);
        std::sort(options.begin(), options.end(),
                  [](const Connection& a, const Connection& b) { return a.cost < b.cost; });

        for (const Connection& option : options) {
            if (option.cost >= best.cost) {
                break;
            }

            if (Environment::isDubinsPathInBounds(tree.tree.points[node],
											      option.end,
                                                  option.option)) {
                best = option;
                break;
            }
        }
    }

    return best;
}

std::vector<RRTPoint> RRT::goalEndpoints(int cur_goal_idx) const {
    std::vector<RRTPoint> ends;
    ends.reserve(goal_angles[cur_goal_idx].size());

    for (const double angle : goal_angles[cur_goal_idx]) {
        ends.emplace_back(goals[cur_goal_idx], angle);
    }

    return ends;
}

bool RRT::connectToGoal(int cur_goal_idx) {
    // TODO : max_paths_checked should be rearchitected
    const Connection connection = bestConnection(goalEndpoints(cur_goal_idx));

    if (!connection.isValid()) {
        return false;
    }

    commitConnection(connection, cur_goal_idx);
    return true;
}

void RRT::commitConnection(const Connection& connection, int cur_goal_idx) {
    const NodeId goal_node = tree.tree.size;
    tree.addSample(connection.anchor, connection.end, connection.option);

    legs.push_back({tree.getStart(),
					tree.findPathToNode(goal_node),
					connection.cost,
					cur_goal_idx});

    // the goal becomes the root of a fresh tree for the next waypoint
    tree.setCurrentHead(connection.end);
}

std::vector<XYZCoord> RRT::buildFlightPath(const Leg& leg) const {
    std::vector<XYZCoord> path = Dubins::generatePath(leg.start, leg.segments);

    if (path.empty()) {
        return path;
    }

    // the leg is flown from the waypoint behind the one it lands on
    const double start_height 	   = goals[leg.goal_idx - 1].z;
    const double height_difference = goals[leg.goal_idx].z - start_height;

	// since our points are not evenly spaced, we have to account for distance
	// when doing altitude transitions. This is a misestimate
    XYZCoord previous = leg.start.coord;
    double total_distance = 0;

    for (XYZCoord& point : path) {
        total_distance += std::hypot(point.x - previous.x, point.y - previous.y);
        previous 	    = point;
        point.z 		= total_distance; // distance flown in path
    }

	// ASSUMPTION: PATH IS NOT A BUNCH OF POINTS ON TOP OF EACH OTHER
    for (XYZCoord& point : path) {
        const double ratio = point.z / total_distance;
        point.z = start_height + height_difference * ratio;
    }

    return path;
}
