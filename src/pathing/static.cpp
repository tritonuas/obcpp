#include "pathing/static.hpp"

#include <httplib.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "core/mission_state.hpp"
#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "pathing/plotting.hpp"
#include "pathing/tree.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

RRT::RRT(RRTPoint start, std::vector<XYZCoord> goals, int iterations_per_waypoint,
         double search_radius, double rewire_radius, Polygon bounds, std::vector<Polygon> obstacles,
         RRTConfig config)
    : iterations_per_waypoint(iterations_per_waypoint),
      search_radius(search_radius),
      rewire_radius(rewire_radius),
      tree(start, Environment(bounds, {}, goals, obstacles),
           Dubins(TURNING_RADIUS, POINT_SEPARATION)),
      config(config) {}

void RRT::run() {
    /*
     * RRT algorithm
     * - Treats each waypoint as a goal, DOES NOT reuse trees between waypoints,
     *    basically calls RRT for each waypoint
     * - For Each Waypoint
     *  - Tries to connect directly to the goal
     *  - If it can't, it runs the RRT algorithm
     *      - Attempts to converge based on epoch intervals
     *      - If it can't, it connects to the goal with whatever it has
     */
    const int total_goals = tree.getAirspace().getNumGoals();

    for (int current_goal_index = 0; current_goal_index < total_goals; current_goal_index++) {
        // tries to connect directly to the goal
        if (connectToGoal(current_goal_index)) {
            continue;
        }

        // run the RRT algorithm if it can not connect
        RRTIteration(iterations_per_waypoint, current_goal_index);
    }
}

std::vector<XYZCoord> RRT::getPointsToGoal() const { return tree.getPathToGoal(); }

bool RRT::RRTIteration(int tries, int current_goal_index) {
    const int epoch_interval = tries / NUM_EPOCHS;
    int current_epoch = epoch_interval;

    RRTNode *goal_node = nullptr;

    for (int i = 0; i < tries; i++) {
        if (i == current_epoch) {
            // generates a new node (not connect), and adds and breaks if it is
            // within X% of the last generation
            if (epochEvaluation(goal_node, current_goal_index)) {
                return true;
            }

            current_epoch += epoch_interval;
        }
        // generate a sample point
        const RRTPoint sample = generateSamplePoint();

        // returns all dubins options from the tree to the sample
        const std::vector<std::pair<RRTNode *, RRTOption>> &options =
            tree.pathingOptions(sample, config.point_fetch_method);

        // returns true if the node is successfully added to the tree
        RRTNode *new_node = parseOptions(options, sample);

        if (new_node != nullptr && config.optimize) {
            optimizeTree(new_node);
        }
    }

    // frees memory
    delete (goal_node);
    if (!connectToGoal(current_goal_index)) {
        loguru::set_thread_name("Static Pathing");
        LOG_F(WARNING, "Failed to connect to goal on iteration: [%s]. Trying again...",
              std::to_string(current_goal_index).c_str());

        if (!config.allowed_to_skip_waypoints &&
            !connectToGoal(current_goal_index, std::numeric_limits<int>::max())) {
            // will always return true (unless it turns into a pseudo-infinite loop)
            return RRTIteration(tries, current_goal_index);
        } else {
            return false;
        }
    }

    return true;
}

bool RRT::epochEvaluation(RRTNode *goal_node, int current_goal_index) {
    // If a single epoch has not been passed, mark this goal as the first
    // benchmark.
    if (goal_node == nullptr) {
        goal_node = sampleToGoal(current_goal_index, TOTAL_OPTIONS_FOR_GOAL_CONNECTION);
        return false;
    }

    RRTNode *new_node = sampleToGoal(current_goal_index, TOTAL_OPTIONS_FOR_GOAL_CONNECTION);

    if (new_node == nullptr) {
        return false;
    }

    delete (goal_node);
    goal_node = new_node;

    // If the new node is within ~X% of the goal, then we are done.
    // It should be impossible for new_node to be more inefficient than
    // goal_node as it uses a superset of the tree goal_node used
    if (new_node->getCost() < EPOCH_TEST_MARGIN * goal_node->getCost()) {
        return false;
    }

    tree.addNode(new_node->getParent(), new_node);
    tree.setCurrentHead(new_node);
    return true;
}

RRTPoint RRT::generateSamplePoint() const { return tree.getRandomPoint(search_radius); }

std::vector<std::pair<RRTPoint, std::pair<RRTNode *, RRTOption>>> RRT::getOptionsToGoal(
    int current_goal_index, int total_options) const {
    // attempts to connect to the goal, should always connect
    std::vector<RRTPoint> goal_points;

    // Generates goal specific points based on current Waypoints and list og
    // Angles
    for (const double angle : angles) {
        const XYZCoord &goal = tree.getAirspace().getGoal(current_goal_index);
        goal_points.push_back(RRTPoint(goal, angle));
    }

    // RRTPoint is the goal that is to be connected
    // RRTNode is the node in the tree that is the anchor
    // RRTOPtion Node-->Point
    std::vector<std::pair<RRTPoint, std::pair<RRTNode *, RRTOption>>> all_options;

    // limit amount of options to sort, defined in constants.hpp
    const int NUMBER_OPTIONS_EACH = total_options / angles.size();

    // gets all options for each of the goals, and puts them into a unified list
    // TODO ? maybe better for a max heap?
    for (const RRTPoint &goal : goal_points) {
        const std::vector<std::pair<RRTNode *, RRTOption>> &options =
            // For now, we use optimal pathing
            tree.pathingOptions(goal, POINT_FETCH_METHODS::NONE, NUMBER_OPTIONS_EACH);

        for (const auto &[node, option] : options) {
            all_options.push_back({goal, {node, option}});
        }
    }

    std::sort(all_options.begin(), all_options.end(), [](const auto &a, const auto &b) {
        auto &[a_goal, a_paths] = a;
        auto &[a_node, a_option] = a_paths;
        auto &[b_goal, b_paths] = b;
        auto &[b_node, b_option] = b_paths;
        return a_option.length + a_node->getCost() < b_option.length + b_node->getCost();
    });

    return all_options;
}

RRTNode *RRT::sampleToGoal(int current_goal_index, int total_options) const {
    // gets all options for each of the goals
    const auto &all_options = getOptionsToGoal(current_goal_index, total_options);

    // <RRTPoint GOAL, {RRTNode* ANCHOR, RRTOption} >
    for (const auto &[goal, pair] : all_options) {
        auto &[anchor_node, option] = pair;

        RRTNode *new_node = tree.generateNode(anchor_node, goal, option);

        if (new_node != nullptr) {
            return new_node;
        }
    }

    return nullptr;
}

bool RRT::connectToGoal(int current_goal_index, int total_options) {
    RRTNode *goal_node = sampleToGoal(current_goal_index, total_options);

    if (goal_node == nullptr) {
        return false;
    }

    // sets the new head
    tree.addNode(goal_node->getParent(), goal_node);
    tree.setCurrentHead(goal_node);
    return true;
}

RRTNode *RRT::parseOptions(const std::vector<std::pair<RRTNode *, RRTOption>> &options,
                           const RRTPoint &sample) {
    for (auto &[node, option] : options) {
        /*
         *  stop if
         *  1. the node is null
         *  2. the node is the same as the sample
         *
         *  The idea is that any further options will have the same if not more
         * issues
         *
         * This shouldn't ever happen?
         */
        // if (node == nullptr || node->getPoint() == sample) {
        //     return nullptr;

        // else, attempt to add the node to the tree
        RRTNode *sucessful_addition = tree.addSample(node, sample, option);

        if (sucessful_addition != nullptr) {
            return sucessful_addition;
        }
    }

    return nullptr;
}

void RRT::optimizeTree(RRTNode *sample) { tree.RRTStar(sample, rewire_radius); }

std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state) {
    // first waypoint is start
    RRTPoint start(state->config.getWaypoints().front(), 0);

    // the other waypoitns is the goals
    if (state->config.getWaypoints().size() < 2) {
        loguru::set_thread_name("Static Pathing");
        LOG_F(ERROR, "Not enough waypoints to generate a path, required 2+, existing waypoints: %s",
              std::to_string(state->config.getWaypoints().size()).c_str());
        return {};
    }

    std::vector<XYZCoord> goals;

    // Copy elements (reference) from the second element to the last element of source into
    // destination all other methods of copying over crash???
    for (int i = 1; i < state->config.getWaypoints().size(); i++) {
        goals.emplace_back(state->config.getWaypoints()[i]);
    }

    RRT rrt(start, goals, ITERATIONS_PER_WAYPOINT, SEARCH_RADIUS, REWIRE_RADIUS,
            state->config.getFlightBoundary(), {}, RRTConfig{true, POINT_FETCH_METHODS::NONE});

    rrt.run();

    std::vector<XYZCoord> path = rrt.getPointsToGoal();
    std::vector<GPSCoord> output_coords;
    int count = 0;

    for (XYZCoord wpt : path) {
        output_coords.push_back(state->getCartesianConverter().value().toLatLng(wpt));
    }

    return output_coords;
}

AirdropSearch::AirdropSearch(const RRTPoint &start, double scan_radius, Polygon bounds,
                             Polygon airdrop_zone, std::vector<Polygon> obstacles,
                             AirdropSearchConfig config)
    : start(start),
      scan_radius(scan_radius),
      airspace(Environment(bounds, airdrop_zone, {}, obstacles)),
      dubins(Dubins(TURNING_RADIUS, POINT_SEPARATION)),
      config(config) {}

std::vector<XYZCoord> AirdropSearch::run() const {
    if (!config.optimize) {
        // generates the endpoints for the lines (including headings)
        std::vector<RRTPoint> waypoints =
            airspace.getAirdropWaypoints(scan_radius, config.one_way, config.vertical);

        // generates the path connecting the q
        std::vector<XYZCoord> path;
        RRTPoint current = start;
        for (const RRTPoint & waypoint: waypoints) {
            std::vector<XYZCoord> dubins_path = dubins.dubinsPath(current, waypoint);
            path.insert(path.end(), dubins_path.begin() + 1, dubins_path.end());
            current = waypoint;
        }

        return path;
    }

    // if optimizing, we store dubins options and then compare lengths

    /*
     * The order of paths
     * [0] - alt, vertical
     * [1] - alt, horizontal
     * [2] - one_way, vertical
     * [3] - one_way, horizontal
     */

    std::vector<std::pair<bool, bool>> configs = {
        {false, true}, {false, false}, {true, true}, {true, false}};

    std::vector<std::vector<RRTOption>> dubins_paths;

    // generates the endpoints for the lines (including headings)
    for (auto &config : configs) {
        std::vector<RRTPoint> waypoints =
            airspace.getAirdropWaypoints(scan_radius, config.first, config.second);

        // generates the path connecting the q
        std::vector<RRTOption> current_dubins_path;
        RRTPoint current = start;
        for (const RRTPoint& waypoint : waypoints) {
            RRTOption dubins_path = dubins.bestOption(current, waypoint);
            current_dubins_path.push_back(dubins_path);
            current = waypoint;
        }

        dubins_paths.push_back(current_dubins_path);
    }

    // finds the shortest path
    int shortest_path_index = 0;
    double shortest_length = std::numeric_limits<double>::max();
    for (int i = 0; i < dubins_paths.size(); i++) {
        double length = 0;
        for (auto &option : dubins_paths[i]) {
            length += option.length;
        }

        if (length < shortest_length) {
            shortest_length = length;
            shortest_path_index = i;
        }
    }

    // actually makes the path
    std::vector<XYZCoord> path;
    RRTPoint current = start;

    // gets the path
    std::vector<RRTPoint> waypoints = airspace.getAirdropWaypoints(
        scan_radius, configs[shortest_path_index].first, configs[shortest_path_index].second);

    // initial path to the region is a special case, so we deal with it individually
    std::vector<XYZCoord> init_path = dubins
                       .generatePoints(current, waypoints[0],
                                       dubins_paths[shortest_path_index][0].dubins_path,
                                       dubins_paths[shortest_path_index][0].has_straight);
    path.insert(path.end(), init_path.begin(), init_path.end());

    // go through all the waypoints
    for (int i = 0; i < dubins_paths[shortest_path_index].size() - 1; i++) {
        std::vector<XYZCoord> path_coordinates = dubins.generatePoints(
            waypoints[i], waypoints[i + 1], dubins_paths[shortest_path_index][i + 1].dubins_path,
            dubins_paths[shortest_path_index][i + 1].has_straight);
        path.insert(path.end(), path_coordinates.begin() + 1, path_coordinates.end());
    }

    return path;
}
