#ifndef INCLUDE_PATHING_STATIC_HPP_
#define INCLUDE_PATHING_STATIC_HPP_

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

class RRT {
 public:
    RRT(RRTPoint start, std::vector<XYZCoord> goals, int iterations_per_waypoint,
        double search_radius, double rewire_radius, Polygon bounds,
        std::vector<Polygon> obstacles = {},
        OptimizationOptions options = {true, PATH_OPTIONS::NEAREST})

        : iterations_per_waypoint(iterations_per_waypoint),
          search_radius(search_radius),
          rewire_radius(rewire_radius),
          point_fetch_choice(options.path_option),
          tree(start, Environment(bounds, goals, obstacles),
               Dubins(TURNING_RADIUS, POINT_SEPARATION)),
          optimize(options.optimize) {}

    /**
     * RRT algorithm
     *
     * TODO - do all iterations to try to find the most efficient path?
     *  - maybe do the tolarance as stright distance / num iterations
     *  - not literally that function, but something that gets more leniant the
     * more iterations there are
     */
    void run() {
        const int total_goals = tree.getAirspace().getNumGoals();

        for (int current_goal_index = 0; current_goal_index < total_goals; current_goal_index++) {
            // tries to connect directly to the goal
            if (connectToGoal(current_goal_index)) {
                continue;
            }

            // run the RRT algorithm if it can not connect
            RRTIteration(iterations_per_waypoint, current_goal_index);

            // connect to the goal after RRT is finished
        }
    }

    /**
     * returns a continuous path of points to the goal
     *
     * @return  ==> list of 2-vectors to the goal region
     */
    std::vector<XYZCoord> getPointsToGoal() { return tree.getPathToGoal(); }

 private:
    RRTTree tree;

    const bool optimize;
    const int iterations_per_waypoint;
    const double search_radius;
    const double rewire_radius;
    const PATH_OPTIONS point_fetch_choice;

    // the different of final approaches to the goal
    // yes, this is the default unit circle diagram used in High-School
    const std::vector<double> angles = {
        0,
        M_PI / 6,
        M_PI / 4,
        M_PI / 3,
        M_PI / 2,
        2 * M_PI / 3,
        3 * M_PI / 4,
        5 * M_PI / 6,
        M_PI,
        7 * M_PI / 6,
        5 * M_PI / 4,
        4 * M_PI / 3,
        3 * M_PI / 2,
        5 * M_PI / 3,
        7 * M_PI / 4,
        11 * M_PI / 6,
    };

    /**
     * Does a single iteration of the RRT(star) algoritm to connect two waypoints
     *
     * @param tries ==> number of points it attempts to sample
     */
    void RRTIteration(const int tries, const int current_goal_index) {

        int epoch_interval = tries / 5;
        int current_epoch = epoch_interval;

        double distance = std::numeric_limits<double>::infinity();
        int angle = 0;


        for (int i = 0; i < tries; i++) {

            if (i == current_epoch) {
                current_epoch += epoch_interval;
            }
            // generate a sample point
            const RRTPoint sample = generateSamplePoint();

            // returns all dubins options from the tree to the sample
            const std::vector<std::pair<RRTNode *, RRTOption>> &options =
                tree.pathingOptions(sample);

            // returns true if the node is successfully added to the tree
            RRTNode *new_node = parseOptions(options, sample);

            if (new_node != nullptr && optimize) {
                optimizeTree(new_node);
            }
        }

        connectToGoal(current_goal_index);
    }

    /**
     * Generates a random point in the airspace, with a bias towards the goal
     * if the random number is less than the goal bias, it just returns the goal
     * to see if we can directly connect to it
     */
    RRTPoint generateSamplePoint() { return tree.getRandomPoint(search_radius); }

    /**
     * Connects to the goal after RRT is finished
     *
     * @param current_goal_index    ==> index of the goal that we are trying to
     * connect to
     * @return                      ==> pointer to the node if it was added,
     * nullptr otherwise
     */
    bool connectToGoal(int current_goal_index) {
        // attempts to connect to the goal, should always connect
        std::vector<RRTPoint> goal_points;
        for (const double angle : angles) {
            const XYZCoord goal = tree.getAirspace().getGoal(current_goal_index);
            goal_points.push_back(RRTPoint(goal, angle));
        }

        // RRTPoint is the goal that is to be connected
        // RRTNode is the node in the tree that is the anchor
        // RRTOPtion Node-->Point
        std::vector<std::pair<RRTPoint, std::pair<RRTNode *, RRTOption>>> all_options;

        const int NUMBER_OPTIONS_TOTAL = 512;
        const int NUMBER_OPTIONS_EACH = NUMBER_OPTIONS_TOTAL / angles.size();
        for (const RRTPoint &goal : goal_points) {
            const std::vector<std::pair<RRTNode *, RRTOption>> &options =
                tree.pathingOptions(goal, NUMBER_OPTIONS_EACH);

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

        for (const auto &[goal, pair] : all_options) {
            auto &[anchor_node, option] = pair;

            RRTNode *new_node = tree.addSample(anchor_node, goal, option);

            if (new_node != nullptr) {
                // print out coordinate of new_node
                tree.setCurrentHead(new_node);
                return true;
            }
        }

        return false;
    }

    /**
     * Goes through generated options to try to connect the sample to the tree
     *
     * @param options   ==> list of options to connect the sample to the tree
     * @param sample    ==> sampled point
     * @return          ==> whether or not the sample was successfully added to
     * the tree (nullptr if not added)
     */
    RRTNode *parseOptions(const std::vector<std::pair<RRTNode *, RRTOption>> &options,
                          const RRTPoint &sample) {
        // directly putting options.size inside the min function angers the compiler???
        const int elements = options.size();
        const int stop_condition = std::min(MAX_DUBINS_OPTIONS_TO_PARSE, elements);

        for (int index = 0; index < stop_condition; index++) {
            const auto &[node, option] = options[index];

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

    /**
     * Rewires the tree by finding paths that are more efficintly routed through
     * the sample. Only searches for nodes a specific radius around the sample
     * to reduce computational expense
     *
     * @param sample    ==> sampled point
     */
    void optimizeTree(RRTNode *sample) { tree.RRTStar(sample, rewire_radius); }
};

std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state);

#endif  // INCLUDE_PATHING_STATIC_HPP_
