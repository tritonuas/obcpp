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

        : num_iterations{num_iterations},
          goal_bias{goal_bias},
          search_radius{search_radius},
          rewire_radius{rewire_radius},
          tree{start, Environment{bounds, goal, tolerance_to_goal},
               Dubins{TURNING_RADIUS, POINT_SEPARATION}} {}

    // choose a random point from free space, not nessisarily this
    RRTPoint generateSamplePoint() {
        if (random(0, 1) < goal_bias) {
            return RRTPoint{tree.getGoal().coord, random(0, TWO_PI)};
        }

        return tree.getRandomPoint(search_radius);
    }

    // /**
    //  * RRT*
    //  */
    // void run() {
    //     for (int _; _ < num_iterations; _++) {
    //         if (tree.getAirspace().isGoalFound()) {
    //             break;
    //         }

    //         RRTPoint sample = generateSamplePoint();

    //         std::vector<std::pair<RRTNode *, RRTOption>> options = tree.pathingOptions(sample);

    //         // [TODO] add rest of function
    //         Validity validity = parseOptions(&options, sample);

    //         switch (validity) {
    //             case INVALID:
    //                 continue;  // test propertly
    //             case GOAL:
    //                 _found_goal = true;
    //                 break;
    //             case VALID:
    //                 break;  // test properly
    //         }
    //     }
    // }

    // /**
    //  * ?????????
    //  */
    // bool parseOptions(std::vector<std::pair<RRTNode *, RRTOption>> *options,
    //                   RRTPoint &sample) {
    //     for (const auto &[node, option] : *options) {
    //         // if the node is the sample, return (preventes loops)
    //         if (node->getPoint() == sample) {
    //             return false;  // invalid
    //         }

    //         // else, add the node to the
    //         bool sucessful_addition = tree.addNode(node, sample);

    //         // for clarity, sample is not used beyond this point. It is now the
    //         // new_node
    //         optimizeTree(&new_node);

    //         // if the new node is within the tolerance of the goal, return ???
    //         if (inGoalRegion(new_node.getPoint())) {
    //             _found_goal = true;
    //             _goal_node = &new_node;
    //             return GOAL;
    //         }

    //         return VALID;
    //     }
    // }

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

    // /**
    //  * returns a continuous path of points to the goal
    //  *
    //  * @return  ==> list of 2-vectors to the goal region
    //  */
    // std::vector<XYZCoord> getPointsGoal() { return getPointsForPath(getGoalPath()); }

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

 private:
    RRTTree tree;

    int num_iterations;
    double goal_bias;
    double search_radius;
    double rewire_radius;
};

#endif  // INCLUDE_PATHING_STATIC_HPP_
