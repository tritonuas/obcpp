#ifndef INCLUDE_PATHING_STATIC_HPP_
#define INCLUDE_PATHING_STATIC_HPP_

#include <httplib.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "pathing/plotting.hpp"
#include "pathing/tree.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

// std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state) {
//     // do pathing here
//     return {};
// }

/*
    TODO - 
        1] get rid of the goal bias, will sample goal before and after iterations
        2] sample the goal before and after iterations
*/
class RRT {
 public:
    RRT(RRTPoint start, std::vector<RRTPoint> goals, int num_iterations, double goal_bias,
        double search_radius, double tolerance_to_goal, double rewire_radius, Polygon bounds)

        : num_iterations(num_iterations),
          goal_bias(goal_bias),
          search_radius(search_radius),
          rewire_radius(rewire_radius),
          tree(start, Environment(bounds, goals, tolerance_to_goal),
               Dubins(TURNING_RADIUS, POINT_SEPARATION)) {}

    /**
     * Generates a random point in the airspace, with a bias towards the goal
     * if the random number is less than the goal bias, it just returns the goal to see if we can
     * directly connect to it
     */
    RRTPoint generateSamplePoint() {
        return tree.getRandomPoint(search_radius, random(0.0, 1.0) < goal_bias);
    }

    /**
     * RRT algorithm
     *
     * TODO - do all iterations to try to find the most efficient path?
     *  - maybe do the tolarance as stright distance / num iterations
     *  - not literally that function, but something that gets more leniant the more iterations
     * there are
     */
    void run() {
        // gets the goal coordinates from the environment
        std::vector<XYZCoord> goal_coords;
        for (const RRTPoint &goal : tree.getAirspace().goals) {
            goal_coords.push_back(goal.coord);
        }

        PathingPlot plotter("pathing_output", tree.getAirspace().valid_region, {}, goal_coords);
        int total_goals = tree.getAirspace().getNumGoals();
        int tries = num_iterations / total_goals;
        int next_goal = 1;

        for (int current_goal_index = 0; current_goal_index < total_goals; current_goal_index++) {
            // base case ==> if the goal is found, stop
            for (int _ = 0; _ < tries; _++) {
                // if (tree.getAirspace().isPathComplete()) {
                //     next_goal++;
                //     break;
                // }

                // generate a sample point
                const RRTPoint sample = generateSamplePoint();

                // returns all dubins options from the tree to the sample
                std::vector<std::pair<RRTNode *, RRTOption>> options = tree.pathingOptions(sample);

                // returns true if the node is successfully added to the tree
                RRTNode *new_node = parseOptions(options, sample, current_goal_index);

                if (new_node != nullptr) {
                    Polyline path =
                        tree.edge_map.at(std::make_pair(new_node->getParent(), new_node)).getPath();
                    plotter.addIntermediatePolyline(path);
                    optimizeTree(new_node);
                }
            }

            // changes the current head to make the tree searching more efficient
            tree.changeCurrentHead(current_goal_index);
        }

        plotter.output("test_animated", PathOutputType::ANIMATED);
    }

    /**
     * Goes through generated options to try to connect the sample to the tree
     *
     * @param options   ==> list of options to connect the sample to the tree
     * @param sample    ==> sampled point
     * @return          ==> whether or not the sample was successfully added to the tree (nullptr if
     * not added)
     */
    RRTNode *parseOptions(const std::vector<std::pair<RRTNode *, RRTOption>> &options,
                          const RRTPoint &sample, int goal_index) {
        for (const auto &[node, option] : options) {
            /*
             *  stop if
             *  1. the node is null
             *  2. the node is the same as the sample
             *  3. the option length is not a number
             *
             *  The idea is that any further options will have the same if not more issues
             *  [TODO] - how is it possible that option.length is not a number??
             *
             */
            if (node == nullptr || node->getPoint() == sample || std::isnan(option.length)) {
                return nullptr;
            }

            // else, add the node to the
            RRTNode *sucessful_addition = tree.addNode(node, sample, option, goal_index);

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

    /**
     * returns a continuous path of points to the goal
     *
     * @return  ==> list of 2-vectors to the goal region
     */
    std::vector<XYZCoord> getPointsGoal(bool without_cache) {
        return tree.getPathToGoal(without_cache);
    }

 private:
    RRTTree tree;

    int num_iterations;
    double goal_bias;
    double search_radius;
    double rewire_radius;
};

#endif  // INCLUDE_PATHING_STATIC_HPP_
