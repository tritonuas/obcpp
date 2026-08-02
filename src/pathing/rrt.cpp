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

std::vector<RRTPoint> goalEndpoints(const XYZCoord& goal, const std::vector<double>& angles) {
    std::vector<RRTPoint> ends;
    ends.reserve(angles.size());

    for (const double angle : angles) {
        ends.emplace_back(goal, angle);
    }

    return ends;
}

RRT::RRT(RRTPoint start) : tree(start) {}

Leg RRT::run(const XYZCoord& goal, const std::vector<double>& angles) {
    const std::vector<RRTPoint> ends = goalEndpoints(goal, angles);

    // tries to connect directly to the goal, which nothing sampling turns up beats
    const Leg direct = connectToGoal(ends);

    if (direct.isValid()) {
        return direct;
    }

    return RRTIteration(goal, ends);
}

Leg RRT::RRTIteration(const XYZCoord& goal, const std::vector<RRTPoint>& ends) {
    std::vector<RRTPoint> sample(1);

    for (NodeId _ = 0; _ < ITERATIONS_PER_WAYPOINT; _++) {
        sample[0] = RRTPoint(Environment::getRandomPoint(false, goal), random(0, TWO_PI));

        // adds the sample to the tree if there is any way to fly to it
        const Connection connection = bestConnection(sample);

        if (connection.isValid()) {
            tree.addSample(connection.anchor, connection.end, connection.option);
        }
    }

    const Leg leg = connectToGoal(ends);

    if (leg.isValid()) {
        return leg;
    }

    loguru::set_thread_name("Static Pathing");
    LOG_F(WARNING, "Failed to connect to goal at (%f, %f). Trying again...", goal.x, goal.y);

    // throws away the tree that failed, keeping the same starting point
    tree.setCurrentHead(tree.getStart());

    // TODO: possiblility for infinite loop
    return RRTIteration(goal, ends);
}

double RRT::lowerBound(NodeId node, const std::vector<RRTPoint>& ends) const {
    const XYZCoord& anchor = tree.tree.points[node].coord;
    double closest         = std::numeric_limits<double>::infinity();

    for (const RRTPoint& end : ends) {
        closest = std::min(closest, anchor.distanceTo(end.coord));
    }

    return tree.tree.length[node] + closest;
}

void RRT::fillOptions(NodeId node, const std::vector<RRTPoint>& ends) const {
    options.clear();
    const RRTPoint& anchor = tree.tree.points[node];
    const double flown     = tree.tree.length[node];

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

            if (Environment::isDubinsPathInBounds(tree.tree.points[node], option.end,
                                                  option.option)) {
                best = option;
                break;
            }
        }
    }

    return best;
}

Leg RRT::connectToGoal(const std::vector<RRTPoint>& ends) {
    // TODO : max_paths_checked should be rearchitected
    const Connection connection = bestConnection(ends);

    if (!connection.isValid()) {
        return {};
    }

    return commitConnection(connection);
}

Leg RRT::commitConnection(const Connection& connection) {
    const NodeId goal_node = tree.tree.size;
    tree.addSample(connection.anchor, connection.end, connection.option);

    const Leg leg = {tree.getStart(),
                     connection.end,
                     tree.findPathToNode(goal_node),
                     connection.cost};

    // the goal becomes the root of a fresh tree for the next waypoint
    tree.setCurrentHead(connection.end);

    return leg;
}
