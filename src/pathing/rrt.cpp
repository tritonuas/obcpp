#include "pathing/rrt.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
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
        ends.emplace_back(RRTPoint(goal, angle));
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
        const auto [anchor, segment] = bestConnection(sample);

        if (anchor != INVALID_NODE) {
            tree.addSample(anchor, segment);
        }
    }

    const Leg leg = connectToGoal(ends);

    if (leg.isValid()) {
        return leg;
    }

    loguru::set_thread_name("Static Pathing");
    LOG_F(WARNING, "Failed to connect to goal at (%f, %f). Trying again...", goal.x, goal.y);

    // throws away the tree that failed, keeping the same starting point
    reroot(tree.getStart());

    // TODO: possiblility for infinite loop
    return RRTIteration(goal, ends);
}

void RRT::fillOptions(NodeId node, const std::vector<RRTPoint>& ends) const {
    options.clear();
    const RRTPoint anchor = tree.point(node);

    for (const RRTPoint& end : ends) {
        // gets all dubins curves from the given node to the end point
        for (const DubinsPath& path : Dubins::allOptions(anchor, end)) {
            // filters out the options that are not valid
            if (!std::isfinite(path.length)) {
                continue;
            }

            options.push_back({end, path});
        }
    }
}

std::pair<NodeId, PathSegment> RRT::bestConnection(const std::vector<RRTPoint>& ends) const {
    NodeId best_anchor = INVALID_NODE;
    PathSegment best{};
    double best_cost = std::numeric_limits<double>::infinity();

    for (NodeId node = 0; node < tree.size; node++) {
        const RRTPoint anchor = tree.point(node);
        const double flown    = tree.length[node];

        fillOptions(node, ends);

        for (const PathSegment& option : options) {
            const double cost = flown + option.path.length;

            // only paths that would win are worth checking against the airspace
            if (cost >= best_cost) {
                continue;
            }

            if (Environment::isDubinsPathInBounds(anchor, option.end, option.path)) {
                best        = option;
                best_anchor = node;
                best_cost   = cost;
            }
        }
    }

    return {best_anchor, best};
}

Leg RRT::connectToGoal(const std::vector<RRTPoint>& ends) {
    // [TODO] : max_paths_checked should be rearchitected
    //          can use lower bounds for distance + kd tree
    const auto [anchor, segment] = bestConnection(ends);

    if (anchor == INVALID_NODE) {
        return {};
    }

    return commitConnection(anchor, segment);
}

Leg RRT::commitConnection(NodeId anchor, const PathSegment& segment) {
    const NodeId goal_node = tree.size;
    tree.addSample(anchor, segment);

    Leg leg = {tree.getStart(), segment.end, tree.findPathToNode(goal_node),
               tree.length[goal_node]};

    return leg;
}

void RRT::reroot(const RRTPoint& waypoint) { tree.setCurrentHead(waypoint); }
