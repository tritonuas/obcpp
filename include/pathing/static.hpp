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
    TODO - abstract out fnctions
*/
class RRT {
 public:
    RRT(RRTPoint start, std::vector<XYZCoord> goals, int num_iterations, double search_radius,
        double rewire_radius, Polygon bounds)

        : num_iterations(num_iterations),
          search_radius(search_radius),
          rewire_radius(rewire_radius),
          tree(start, Environment(bounds, goals), Dubins(TURNING_RADIUS, POINT_SEPARATION)) {}

    /**
     * Generates a random point in the airspace, with a bias towards the goal
     * if the random number is less than the goal bias, it just returns the goal to see if we can
     * directly connect to it
     */
    RRTPoint generateSamplePoint() { return tree.getRandomPoint(search_radius); }

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
        for (const XYZCoord &goal : tree.getAirspace().goals) {
            goal_coords.push_back(goal);
        }

        PathingPlot plotter("pathing_output", tree.getAirspace().valid_region, {}, goal_coords);
        int total_goals = tree.getAirspace().getNumGoals();
        int tries = num_iterations / total_goals;

        for (int current_goal_index = 0; current_goal_index < total_goals; current_goal_index++) {
            int direct_distance = tree.getGoal(current_goal_index)
                                      .distanceTo(tree.getCurrentHead()->getPoint().coord);

            // try to connect to the goal directly
            // first is the goal version
            // second is the option
            std::vector<std::pair<RRTPoint, RRTOption>> direct_options;
            RRTPoint current_head = tree.getCurrentHead()->getPoint();
            bool direct_success = false;

            for (const double &angle : angles) {
                RRTPoint goal(tree.getAirspace().getGoal(current_goal_index), angle);

                // returns all dubins options from the tree to the sample
                std::vector<RRTOption> options = tree.dubins.allOptions(current_head, goal);

                for (const RRTOption &option : options) {
                    direct_options.emplace_back(std::make_pair(goal, option));
                }
            }

            // sort the options by length
            std::sort(direct_options.begin(), direct_options.end(),
                      [](const std::pair<RRTPoint, RRTOption> &a,
                         const std::pair<RRTPoint, RRTOption> &b) {
                          return compareRRTOptionLength(a.second, b.second);
                      });

            // if the direct distance is within x of the best option, then just connect to the goal
            // TODO get rid of magic number
            for (const auto &[goal, option] : direct_options) {
                if (option.length < 1.2 * direct_distance) {
                    RRTNode *new_node = tree.addNode(tree.getCurrentHead(), goal, option);

                    if (new_node != nullptr) {
                        Polyline path =
                            tree.edge_map.at(std::make_pair(new_node->getParent(), new_node))
                                .getPath();
                        plotter.addIntermediatePolyline(path);
                        tree.setCurrentHead(new_node);
                        direct_success = true;
                        break;
                    }
                } else {
                    break;
                }
            }

            if (direct_success) {
                continue;
            }

            for (int _ = 0; _ < tries; _++) {
                // generate a sample point
                const RRTPoint sample = generateSamplePoint();

                // returns all dubins options from the tree to the sample
                std::vector<std::pair<RRTNode *, RRTOption>> options = tree.pathingOptions(sample);

                // returns true if the node is successfully added to the tree
                RRTNode *new_node = parseOptions(options, sample);

                if (new_node != nullptr) {
                    Polyline path =
                        tree.edge_map.at(std::make_pair(new_node->getParent(), new_node)).getPath();
                    plotter.addIntermediatePolyline(path);
                    optimizeTree(new_node);
                }
            }

            // attempts to connect to the goal, should always connect
            std::vector<RRTPoint> goal_points;
            for (const double angle : angles) {
                goal_points.push_back(
                    RRTPoint(tree.getAirspace().getGoal(current_goal_index), angle));
            }

            std::vector<std::pair<RRTPoint, std::pair<RRTNode *, RRTOption>>> all_options;
            for (const RRTPoint &goal : goal_points) {
                std::vector<std::pair<RRTNode *, RRTOption>> options = tree.pathingOptions(goal);

                for (const auto &[node, option] : options) {
                    all_options.push_back(std::make_pair(goal, std::make_pair(node, option)));
                }
            }

            std::sort(all_options.begin(), all_options.end(), [](const auto &a, const auto &b) {
                return compareRRTOptionLength(a.second.second, b.second.second);
            });

            for (const auto &[goal, pair] : all_options) {
                RRTNode *anchor_node = pair.first;
                RRTOption option = pair.second;

                RRTNode *new_node = tree.addNode(anchor_node, goal, option);

                if (new_node != nullptr) {
                    Polyline path =
                        tree.edge_map.at(std::make_pair(new_node->getParent(), new_node)).getPath();
                    plotter.addIntermediatePolyline(path);
                    tree.setCurrentHead(new_node);
                    break;
                }
            }
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
    std::vector<XYZCoord> getPointsGoal(bool without_cache) {
        return tree.getPathToGoal(without_cache);
    }

 private:
    RRTTree tree;

    int num_iterations;
    double search_radius;
    double rewire_radius;

    const std::vector<double> angles = {0,    M_PI / 4,     M_PI / 2,     3 * M_PI / 4,
                                        M_PI, 5 * M_PI / 4, 3 * M_PI / 2, 7 * M_PI / 4};
};

#endif  // INCLUDE_PATHING_STATIC_HPP_
