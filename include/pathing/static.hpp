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
#include "pathing/tree.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

// std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state) {
//     // do pathing here
//     return {};
// }

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
        // TODO - try for goal directly, or just move towards it
        if (random(0.0, 1.0) < goal_bias) {
            return RRTPoint{tree.getGoal().coord, random(0.0, TWO_PI)};
        }

        return tree.getRandomPoint(search_radius);
    }

    /**
     * RRT algorithm
     *
     * TODO - do all iterations to try to find the most efficient path?
     */
    void run() {
        for (int _ = 0; _ < num_iterations; _++) {
            // base case ==> if the goal is found, stop
            if (tree.getAirspace().isGoalFound()) {
                break;
            }

            // generate a sample point
            const RRTPoint sample = generateSamplePoint();

            // returns all dubins options from the tree to the sample
            std::vector<std::pair<RRTNode *, RRTOption>> options = tree.pathingOptions(sample);

            // returns true if the node is successfully added to the tree
            RRTNode *new_node = parseOptions(options, sample);

            if (new_node != nullptr) {
                optimizeTree(new_node);
            }

            // [TODO] add rest of function
        }
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
                          const RRTPoint &sample) {
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
            RRTNode *sucessful_addition = tree.addNode(node, sample, option);

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
    std::vector<XYZCoord> getPointsGoal() { return tree.getPathToGoal(); }

 private:
    RRTTree tree;

    int num_iterations;
    double goal_bias;
    double search_radius;
    double rewire_radius;
};

#endif  // INCLUDE_PATHING_STATIC_HPP_
