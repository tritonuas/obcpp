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
         OptimizationOptions options)
    : iterations_per_waypoint(iterations_per_waypoint),
      search_radius(search_radius),
      rewire_radius(rewire_radius),

      point_fetch_choice(options.path_option),
      tree(start, Environment(bounds, goals, obstacles), Dubins(TURNING_RADIUS, POINT_SEPARATION)),
      optimize(options.optimize){};

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

void RRT::RRTIteration(int tries, int current_goal_index) {
    int epoch_interval = tries / 5;
    int current_epoch = epoch_interval;

    RRTNode *goal_node = nullptr;

    for (int i = 0; i < tries; i++) {
        if (i == current_epoch) {
            // generates a new node (not connect), and adds and breaks if it is
            // within X% of the last generation
            if (epochEvaluation(goal_node, current_goal_index)) {
                return;
            }

            current_epoch += epoch_interval;
        }
        // generate a sample point
        const RRTPoint sample = generateSamplePoint();

        // returns all dubins options from the tree to the sample
        const std::vector<std::pair<RRTNode *, RRTOption>> &options =
            tree.pathingOptions(sample, point_fetch_choice);

        // returns true if the node is successfully added to the tree
        RRTNode *new_node = parseOptions(options, sample);

        if (new_node != nullptr && optimize) {
            optimizeTree(new_node);
        }
    }

    // frees memory
    delete (goal_node);
    if (!connectToGoal(current_goal_index)) {
        std::cout << "Failed to connect to goal on iteration: [" << current_goal_index << "]"
                  << std::endl;
    }
}

bool RRT::epochEvaluation(RRTNode *goal_node, int current_goal_index) {
    // If a single epoch has not been passed, mark this goal as the first
    // benchmark.
    if (goal_node == nullptr) {
        goal_node = sampleToGoal(current_goal_index);
        return false;
    }

    RRTNode *new_node = sampleToGoal(current_goal_index);

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
    int current_goal_index) const {
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
    const int NUMBER_OPTIONS_EACH = TOTAL_OPTIONS_FOR_GOAL_CONNECTION / angles.size();

    // gets all options for each of the goals, and puts them into a unified list
    // TODO ? maybe better for a max heap?
    for (const RRTPoint &goal : goal_points) {
        const std::vector<std::pair<RRTNode *, RRTOption>> &options =
            // For now, we use optimal pathing
            tree.pathingOptions(goal, PATH_OPTIONS::NONE, NUMBER_OPTIONS_EACH);

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

RRTNode *RRT::sampleToGoal(int current_goal_index) const {
    // gets all options for each of the goals
    const std::vector<std::pair<RRTPoint, std::pair<RRTNode *, RRTOption>>> &all_options =
        getOptionsToGoal(current_goal_index);

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

bool RRT::connectToGoal(int current_goal_index) {
    RRTNode *goal_node = sampleToGoal(current_goal_index);

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
        // }

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
        std::cout << "Not enough waypoints" << std::endl;
        return {};
    }

    std::vector<XYZCoord> goals;

    // Copy elements (reference) from the second element to the last element of source into
    // destination all other methods of copying over crash???
    for (int i = 1; i < state->config.getWaypoints().size(); i++) {
        goals.emplace_back(state->config.getWaypoints()[i]);
    }

    RRT rrt(start, goals, ITERATIONS_PER_WAYPOINT, SEARCH_RADIUS, REWIRE_RADIUS,
            state->config.getFlightBoundary(), {}, OptimizationOptions{true, PATH_OPTIONS::NONE});

    rrt.run();

    std::vector<XYZCoord> path = rrt.getPointsToGoal();
    std::vector<GPSCoord> output_coords;
    int count = 0;

    for (XYZCoord wpt : path) {
        output_coords.push_back(state->getCartesianConverter().value().toLatLng(wpt));
    }

    return output_coords;
}