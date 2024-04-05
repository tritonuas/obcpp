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
        RRTConfig config = {.optimize = false,
                            .point_fetch_method = POINT_FETCH_METHODS::NONE,
                            .allowed_to_skip_waypoints = false});

    /**
     * RRT(-star) algorithm
     *
     * TODO - do all iterations to try to find the most efficient path?
     *  - maybe do the tolarance as stright distance / num iterations
     *  - not literally that function, but something that gets more leniant the
     * more iterations there are
     */
    void run();

    /**
     * returns a continuous path of points to the goal
     *
     * @return  ==> list of 2-vectors to the goal region
     */
    std::vector<XYZCoord> getPointsToGoal() const;

 private:
    // tree stores
    // - the airspace (environment.hpp) that checks valid bounds
    // - the dubins object that generates paths
    // - the nodes that that form the tree
    RRTTree tree;

    /* RRT Config Options */
    const int iterations_per_waypoint;  // number of times to run the RRT algorithm
                                        // for each waypoint
    const double search_radius;         // !!NOT USED!! max radius to move off the tree
    const double rewire_radius;         // ONLY FOR RRT-STAR, max radius from new node to rewire
    const RRTConfig config;             // optimization options

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
     * @return      ==> whether or not the goal was reached
     */
    bool RRTIteration(int tries, int current_goal_index);

    /**
     * Evaluates a certain interval to determine if the algorithm is making
     * meaningful progress. If it isn't, it will simply tell the RRT algoritm to
     * stop.
     */
    bool epochEvaluation(RRTNode *goal_node, int current_goal_index);

    /**
     * Generates a random point in the airspace (uniformly)
     *
     * @return  ==> random point in the airspace
     */
    RRTPoint generateSamplePoint() const;

    /**
     * Gets a sorted list of options to EACH one of the possible goals, defined
     * by the angles we want to connect to
     *
     * @param current_goal_index    ==> index of the goal that we are trying to
     * connect to
     * @param total_options         ==> number of options to try to connect to the goal
     * @return                      ==> list of options to connect to the goal
     *                                  <RRTPoint GOAL, {RRTNode* ANCHOR,
     * RRTOption} >
     */
    std::vector<std::pair<RRTPoint, std::pair<RRTNode *, RRTOption>>> getOptionsToGoal(
        int current_goal_index, int total_options) const;

    /**
     * Tries to get the optimal  node to the goal, which is NOT connected into the
     * tree
     *
     * @param current_goal_index    ==> index of the goal that we are trying to
     * connect to
     * @param total_options         ==> number of options to try to connect to the goal
     * @return                      ==> pointer to the node if one was found,
     * nullptr otherwise
     */
    RRTNode *sampleToGoal(int current_goal_index, int total_options) const;

    /**
     * Connects to the goal after RRT is finished
     *
     * @param current_goal_index    ==> index of the goal that we are trying to
     * connect to
     * @param total_options         ==> number of options to try to connect to the goal
     * @return                      ==> pointer to the node if it was added,
     * nullptr otherwise
     */
    bool connectToGoal(int current_goal_index,
                       int total_options = TOTAL_OPTIONS_FOR_GOAL_CONNECTION);

    /**
     * Goes through generated options to try to connect the sample to the tree
     *
     * @param options   ==> list of options to connect the sample to the tree
     * @param sample    ==> sampled point
     * @return          ==> whether or not the sample was successfully added to
     * the tree (nullptr if not added)
     */
    RRTNode *parseOptions(const std::vector<std::pair<RRTNode *, RRTOption>> &options,
                          const RRTPoint &sample);

    /**
     * Rewires the tree by finding paths that are more efficintly routed through
     * the sample. Only searches for nodes a specific radius around the sample
     * to reduce computational expense
     *
     * @param sample    ==> sampled point
     */
    void optimizeTree(RRTNode *sample);
};

class AirdropSearch {
 public:
    AirdropSearch(RRTPoint start, int scan_radius, Polygon bounds,
                  std::vector<Polygon> obstacles = {})
        : start(start),
          scan_radius(scan_radius),
          airspace(Environment(bounds, {}, obstacles)),
          dubins(Dubins(TURNING_RADIUS, POINT_SEPARATION)) {}

    /**
     * Runs the RRT algorithm to find the optimal path to the goal
     *
     * @return  ==> list of 2-vectors to the goal region
     */
    std::vector<XYZCoord> run() {
        auto bounds = airspace.getBounds();
        auto [x_min, x_max] = bounds.first;
        auto [y_min, y_max] = bounds.second;

        // generates waypoints to search the entire bounds
        std::vector<RRTPoint> waypoints;
        bool fly_down = true;
        for (double x = x_min; x < x_max; x += scan_radius) {
            double angle = fly_down ? 0 : M_PI;

            waypoints.push_back(RRTPoint(XYZCoord(x, y_max, 0), angle));
            waypoints.push_back(RRTPoint(XYZCoord(x, y_min, 0), angle));
        }

        // generates dubins from the start to the waypoints
        std::vector<XYZCoord> path;
        RRTPoint current = start;
        for (auto &waypoint : waypoints) {
            auto dubins_path = dubins.dubinsPath(current, waypoint);
            path.insert(path.end(), dubins_path.begin(), dubins_path.end());
            current = waypoint;
        }

        return path;
    }

 private:
    const int scan_radius;
    const RRTPoint start;
    const Environment airspace;
    const Dubins dubins;
};

std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state);

#endif  // INCLUDE_PATHING_STATIC_HPP_
