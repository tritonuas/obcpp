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
#include "pathing/mission_path.hpp"
#include "pathing/plotting.hpp"
#include "pathing/tree.hpp"
#include "udp_squared/internal/enum.h"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

class RRT {
 public:
    RRT(RRTPoint start, std::vector<XYZCoord> goals, double search_radius, Polygon bounds,
        const OBCConfig &config, std::vector<Polygon> obstacles = {},
        std::vector<double> angles = {});
    RRT(RRTPoint start, std::vector<XYZCoord> goals, double search_radius, Environment airspace,
        const OBCConfig &config, std::vector<double> angles = {});

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
    std::vector<XYZCoord> flight_path;

    // the different of final approaches to the goal
    // yes, this is the default unit circle diagram used in High-School
    std::vector<double> angles = {
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
     * Does the logistical work when found one waypoint to another
     *  - adds the node to the tree
     *  - finds the path
     *      - adds altitude to the path
     *
     * @param goal_node  ==> node to add to the tree
     * @param current_goal_index ==> index of the goal that we are trying to
     */
    void addNodeToTree(RRTNode *goal_node, int current_goal_index);

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

/**
 * Class that performs Coverage-Path_Planning (CPP) over a given polygon
 *
 * Basically draws vertical lines, and the connects them with Dubins paths
 *
 * Limitations
 * - Cannot path through non-convex shapes
 * - Does not check if path is inbounds or not
 *
 * Notes:
 * - this implementation is for fixed wing planes, which is not currently being used. However,
 *   it is kept here because it is very possible we will eventually switch back to it.
 */
class ForwardCoveragePathing {
 public:
    ForwardCoveragePathing(const RRTPoint &start, double scan_radius, Polygon bounds,
                           Polygon airdrop_zone, const OBCConfig &config,
                           std::vector<Polygon> obstacles = {});

    /**
     * Generates a path of parallel lines to cover a given area
     *
     * TODO - optimize dubins to not have to go to each line, rather search every other line then
     * loop back
     *
     * @return  ==> list of 2-vectors describing the path through the aridrop_zone
     */
    std::vector<XYZCoord> run() const;

    /**
     *   The algorithm run if not optimizing the path legnth
     */
    std::vector<XYZCoord> coverageDefault() const;

    /**
     *   The algorithm run if optimizing path length
     */
    std::vector<XYZCoord> coverageOptimal() const;

    /**
     * From a list of dubins paths and waypoints, generate a path
     *
     * @param dubins_options  ==> list of dubins options to connect the waypoints
     * @param waypoints       ==> list of waypoints to connect (always 1 more element than
     * dubins_options)
     */
    std::vector<XYZCoord> generatePath(const std::vector<RRTOption> &dubins_options,
                                       const std::vector<RRTPoint> &waypoints) const;

 private:
    const double scan_radius;    // how far each side of the plane we intend to look (half dist
                                 // between search lines)
    const RRTPoint start;        // start location (doesn't have to be near polygon)
    const Environment airspace;  // information aobut the airspace
    const Dubins dubins;         // dubins object to generate paths
    const AirdropCoverageConfig config;
};

/**
 * Class that performs coverage pathing over a given search area, given that the plane has
 * hovering capabilities and that we want to be taking pictures while hovering over the zone.
 *
 * This outputs a series of XYZ Coordinates which represent a points at which the plane
 * should hover and take a picture.
 *
 * Assumptions:
 * - The drop zone has 4 points which form a rectangle larger than the vision of the camera
 */
class HoverCoveragePathing {
 public:
    explicit HoverCoveragePathing(std::shared_ptr<MissionState> state);

    std::vector<XYZCoord> run();

 private:
    std::shared_ptr<MissionState> state;
    AirdropCoverageConfig config;
    Polygon drop_zone;
};

class AirdropApproachPathing {
 public:
    AirdropApproachPathing(const RRTPoint &start, const XYZCoord &goal, XYZCoord wind,
                           Polygon bounds, const OBCConfig &config,
                           std::vector<Polygon> obstacles = {});
    /**
     * Generates a path to the drop location
     *
     * @return  ==> list of 2-vectors describing the path to the drop location
     */
    std::vector<XYZCoord> run() const;

    /**
     * Generates the vector to the drop location
     */
    RRTPoint getDropLocation() const;

 private:
    const XYZCoord goal;
    const RRTPoint start;
    const Environment airspace;
    const Dubins dubins;
    const OBCConfig config;

    XYZCoord wind;
};

MissionPath generateInitialPath(std::shared_ptr<MissionState> state);

MissionPath generateSearchPath(std::shared_ptr<MissionState> state);

MissionPath generateAirdropApproach(std::shared_ptr<MissionState> state, const GPSCoord &goal);

std::pair<double, double> estimateAreaCoveredAndPathLength(const std::vector<XYZCoord> &goals,
                                                           const Environment &airspace);

std::vector<std::vector<XYZCoord>> generateGoalListDeviations(const std::vector<XYZCoord> &goals,
                                                              XYZCoord deviation_point);

std::vector<std::vector<XYZCoord>> generateRankedNewGoalsList(const std::vector<XYZCoord> &goals,
                                                              const Environment &airspace);

#endif  // INCLUDE_PATHING_STATIC_HPP_
