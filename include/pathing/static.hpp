#ifndef INCLUDE_PATHING_STATIC_HPP_
#define INCLUDE_PATHING_STATIC_HPP_

#include <httplib.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "core/states.hpp"
#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "pathing/tree.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state) {
    // do pathing here
    return {};
}

class RRT {
 public:
    RRT(RRTPoint start, RRTPoint goal, int num_iterations, double goal_bias, double search_radius,
        double tolerance_to_goal, double rewire_radius, Polygon bounds)

        : num_iterations(num_iterations),
          goal_bias(goal_bias),
          search_radius(search_radius),
          rewire_radius(rewire_radius),
          tree(start, Environment(bounds, goal, tolerance_to_goal),
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
            RRTPoint sample = generateSamplePoint();

            // returns all dubins options from the tree to the sample
            std::vector<std::pair<RRTNode *, RRTOption>> options = tree.pathingOptions(sample);

            // returns true if the node is successfully added to the tree
            bool validity = parseOptions(&options, sample);

            // [TODO] add rest of function
        }
    }

    /**
     * Goes through generated options to try to connect the sample to the tree
     *
     * @param options   ==> list of options to connect the sample to the tree
     * @param sample    ==> sampled point
     * @return          ==> whether or not the sample was successfully added to the tree
     */
    bool parseOptions(std::vector<std::pair<RRTNode *, RRTOption>> *options, RRTPoint &sample) {
        for (const auto &[node, option] : *options) {
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
                return false;
            }

            // else, add the node to the
            bool sucessful_addition = tree.addNode(node, sample, option);

            if (sucessful_addition) {
                return true;
            }

            // optimizeTree(&new_node);
        }

        return false;
    }

    // /**
    //  * Rewires the tree by finding paths that are more efficintly routed through
    //  * the sample. Only searches for nodes a specific radius around the sample
    //  * to reduce computational expense
    //  *
    //  * @param sample    ==> sampled point
    //  */
    // void optimizeTree(RRTNode *sample) { optimizeTreeRecursive(_root, nullptr, sample); }

    // /**
    //  *  (RECURSIVE HELPER)
    //  * Rewires the tree by finding paths that are more efficintly routed through
    //  * the sample. Only searches for nodes a specific radius around the sample
    //  * to reduce computational expense
    //  *
    //  * @param node      ==> current node (DFS)
    //  * @param sample    ==> sampled point
    //  */
    // void optimizeTreeRecursive(RRTNode *node, RRTNode *prev_node, RRTNode *sample) {
    //     if (node == nullptr) {
    //         return;
    //     }

    //     for (RRTNode *child : node->getReachable()) {
    //         optimizeTreeRecursive(child, node, sample);
    //     }

    //     // prevents rerouting the root (it shouldn't, but just to be safe)
    //     if (prev_node == nullptr) {
    //         return;
    //     }

    //     // TODO --> optimize the tree

    //     // if the node is not sufficiently close to the sample, dont bother
    //     // optimizing
    //     if (node->getPoint().distanceTo(sample->getPoint()) > _rewire_radius) {
    //         return;
    //     }

    //     // if this node is the sample, return
    //     if (node->getPoint() == sample->getPoint()) {
    //         return;
    //     }

    //     // generate all dubins path from this node
    //     std::vector<RRTOption> options =
    //         _dubins.allOptions(sample->getPoint(), node->getPoint(), true);

    //     for (const RRTOption &option : options) {
    //         // if the option is not possible, don't bother checking
    //         if (option.length == std::numeric_limits<double>::infinity()) {
    //             break;
    //         }

    //         std::vector<XYZCoord> current_path = _dubins.generatePoints(
    //             sample->getPoint(), node->getPoint(), option.dubins_path, option.has_straight);
    //         // the the path is bad, check the next one
    //         if (!validPath(current_path)) {
    //             continue;
    //         }

    //         // if the node is uncompetitive, discard it
    //         if (node->getCost() + option.length >= _tree.getNode(sample->getPoint())->getCost())
    //         {
    //             continue;
    //         }

    //         // else, replace the existing edge with the new one
    //         _tree.rewireEdge(sample, prev_node, node, current_path,
    //                          sample->getCost() + option.length);
    //     }
    // }

    // /**
    //  * Determines whether a given point is close enough to the goal region
    //  * (based on some arbitrary tolarance).
    //  *
    //  * @param sample    ==> sampled point
    //  * @return          ==> whether or not the point is in the goal area
    //  */
    // bool inGoalRegion(const RRTPoint &sample) {
    //     return _goal.distanceTo(sample) <= _tolerance_to_goal;
    // }

    /**
     * returns a continuous path of points to the goal
     *
     * @return  ==> list of 2-vectors to the goal region
     */
    std::vector<XYZCoord> getPointsGoal() { return tree.getPathToGoal(); }

    // /**
    //  * Given a list of edges in the tree, it returns the list that represents
    //  * the path formed by connecting each node by dubins curves.
    //  *
    //  * @param path  ==> list of edges from the root of the tree to the goal
    //  *                  node
    //  * @return      ==> list of points that connects the path through dubins
    //  *                  curves
    //  */
    // std::vector<XYZCoord> getPointsForPath(std::vector<RRTEdge *> path) {
    //     std::vector<XYZCoord> points;

    //     for (RRTEdge *edge : path) {
    //         for (const XYZCoord &point : edge->getPath()) {
    //             points.emplace_back(point);
    //         }
    //     }

    //     return points;
    // }

    // /**
    //  * Finds the path from the start position to the goal area through nodes in
    //  * the tree.
    //  *
    //  * @return  ==> list of edges that connect start to goal nodes
    //  */
    // std::vector<RRTEdge *> getGoalPath() {
    //     std::vector<RRTEdge *> path;
    //     getGoalPathRecursive(_root, &path);
    //     return path;
    // }

    // /**
    //  * (RECURSIVE HELPER)
    //  * Finds the path from the start position to the goal area through nodes in
    //  * the tree.
    //  *
    //  * @param node  ==> the current node (DFS)
    //  * @param path  ==> the list that is filled by this recursive helper
    //  */
    // void getGoalPathRecursive(RRTNode *node, std::vector<RRTEdge *> *path) {
    //     if (node == nullptr) {
    //         return;
    //     }

    //     if (inGoalRegion(node->getPoint())) {
    //         return;
    //     }

    //     for (RRTNode *child : node->getReachable()) {
    //         path->emplace_back(_tree.getEdge(node->getPoint(), child->getPoint()));
    //         getGoalPathRecursive(child, path);
    //     }

    //     path->pop_back();
    // }

    //  private:
    RRTTree tree;

    int num_iterations;
    double goal_bias;
    double search_radius;
    double rewire_radius;
};

#endif  // INCLUDE_PATHING_STATIC_HPP_
