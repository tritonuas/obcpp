#ifndef INCLUDE_PATHING_RRT_HPP_
#define INCLUDE_PATHING_RRT_HPP_

#include <array>
#include <cmath>
#include <limits>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/tree.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

// the different of final approaches to the goal
// yes, this is the default unit circle diagram used in High-School
inline const std::vector<double> DEFAULT_GOAL_ANGLES = {
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
 * A candidate flight from a node already in the tree to some point.
 *
 * The tree only stores nodes that are connected to it, so a connection that has
 * not been committed yet is described by the anchor it would hang off of and the
 * dubins path that gets there. The point may be a sample or a goal, the search
 * that finds it does not care which.
 */
struct Connection {
    NodeId anchor = INVALID_NODE;  // node in the tree the path starts at
    RRTPoint end{};                // the vector the path lands on
    RRTOption option{};            // dubins path anchor --> end
    double cost = std::numeric_limits<double>::infinity();  // path length from the root to the end

    inline bool isValid() const { return anchor != INVALID_NODE; }
};

/**
 * One leg of the mission, flown from a waypoint to the one behind it.
 *
 * The tree that found the leg is thrown away as soon as the waypoint is reached,
 * so what it settled on is kept here instead. That is enough to say how long the
 * leg is, which is all a caller comparing two missions needs -- the points along
 * it are not worth flying until one of them has been picked.
 */
struct Leg {
    RRTPoint start;                     // the waypoint the leg is flown from
    std::vector<PathSegment> segments;  // the dubins paths flown, in order
    double length = 0;                  // the ground the leg covers
    int goal_idx = 0;                   // index of the goal the leg lands on
};

class RRT {
 public:
    // tree stores the nodes that form the tree
    RRTTree tree;

    /*
     * The waypoints to path through, in order, the first of which is where the
     * plane starts out. A leg is always flown from the waypoint behind it, so
     * the start being one of them is what keeps the first leg from being a
     * special case.
     */
    const std::vector<XYZCoord> goals;

    /*
     * The final approach angles each goal may be reached at, one set per goal.
     * The cheapest of them wins, so a caller that does not care which way a goal
     * is approached hands over every angle, and one that does -- coverage pathing
     * does, as a scan line only sweeps the ground it is meant to if it is flown
     * along its own direction -- hands over the single angle it wants.
     *
     * The first set is the heading the plane is already flying, as the goal it is
     * sitting on is flown from rather than reached.
     */
    const std::vector<std::vector<double>> goal_angles;

    // the legs of the mission that have been searched out so far
    std::vector<Leg> legs;

    // the points flown along those legs, once they have been generated
    std::vector<XYZCoord> flight_path;

    /*
     * Scratch space for bestConnection, which runs once for every sample RRT takes
     * and would otherwise lay all of this out again each time. It is not state --
     * nothing may read it between calls, and every call fills it before it reads it.
     */
    mutable std::array<double, TREE_CAPACITY> bounds;    // by node, the cheapest flight through it
    mutable std::array<NodeId, TREE_CAPACITY> frontier;  // the order the nodes are looked at in
    mutable std::vector<Connection> options;             // the paths out of the node being expanded


    /**
     * @param[in] goals         ==> the waypoints to fly through, in order, the
     *                              first of which is where the plane already is
     * @param[in] start_angle   ==> the heading the plane is flying at right now
     * @param[in] goal_angles   ==> the angles each goal may be approached at, one
     *                              set per goal. A goal that has to be flown at
     *                              one particular heading is a set of one. The
     *                              set for the first goal is not read, the plane
     *                              is already sitting on it at start_angle.
     */
    RRT(std::vector<XYZCoord> goals, double start_angle,
        std::vector<std::vector<double>> goal_angles);

    /**
     * @param[in] goals         ==> the waypoints to fly through, in order, the
     *                              first of which is where the plane already is
     * @param[in] start_angle   ==> the heading the plane is flying at right now
     * @param[in] angles        ==> the angles every goal may be approached at
     */
    RRT(std::vector<XYZCoord> goals, double start_angle, std::vector<double> angles = {});

    /**
     * RRT algorithm -- searches out the mission and then flies it
     */
    void run();

    /**
     * Searches out the dubins paths that fly the mission, and nothing more
     *
     * How long the mission is falls out of this, so a caller weighing one against
     * another can stop here and only pay for the points of the one it flies.
     *
     * TODO - do all iterations to try to find the most efficient path?
     *  - maybe do the tolarance as stright distance / num iterations
     *  - not literally that function, but something that gets more leniant the
     * more iterations there are
     */
    void generateDubinsOptions();

    /**
     * Flies the legs, which is the only thing that generates points
     */
    void generateFlightPoints();

    /**
     * The ground the legs found so far cover
     *
     * Available as soon as the dubins paths are, the points do not have to have
     * been generated.
     *
     * @return  ==> the length of every leg flown, added up
     */
    double pathLength() const;

    /**
     * returns a continuous path of points to the goal
     *
     * @return  ==> list of 2-vectors to the goal region
     */
    std::vector<XYZCoord> getPointsToGoal() const;

    /**
     * Does a single iteration of the RRT(star) algoritm to connect two waypoints
     *
     * @return  ==> whether or not the goal was reached
     */
    bool RRTIteration(uint8_t cur_goal_idx);

    /**
     * The cheapest a flight through a node could possibly be
     *
     * A dubins path is never shorter than the straight line between the two
     * points it connects, so the flight has to cover at least the distance
     * already flown to reach the node plus that straight line.
     *
     * @param[in] node  ==> node in the tree the flight would go through
     * @param[in] ends  ==> the points being pathed to
     * @return  ==> lower bound on the cost of any connection from the node
     */
    double lowerBound(NodeId node, const std::vector<RRTPoint> &ends) const;

    /**
     * Appends every dubins path that exists from a single node to each of the
     * given points
     *
     * @param[in] node  ==> node in the tree to path from
     * @param[in] ends  ==> the points to path to
     */
    void fillOptions(NodeId node, const std::vector<RRTPoint> &ends) const;

    /**
     * Finds the cheapest flyable connection from the tree to any of the given
     * points, which is NOT added into the tree
     *
     * Nodes are visited in order of the cheapest flight they could possibly
     * offer, so the search stops as soon as that bound passes the best
     * connection it already holds -- the rest of the tree cannot beat it, and
     * pathing from it would be wasted work.
     *
     * @param[in] ends              ==> the points to path to
     * @param[in] max_paths_checked ==> how many paths may be checked before
     *                                  giving up
     * @return  ==> the connection if one was found, an invalid connection
     *              otherwise
     */
    Connection bestConnection(const std::vector<RRTPoint> &ends, int max_paths_checked) const;

    /**
     * The points a goal can be reached at, one for every angle it may be
     * approached at -- which is a single point when the caller pinned it down
     *
     * @param[in] cur_goal_idx  ==> index of the goal that we are trying to
     *                              connect to
     * @return  ==> the goal, at each of the approach angles
     */
    std::vector<RRTPoint> goalEndpoints(int cur_goal_idx) const;

    /**
     * Connects to the goal after RRT is finished
     *
     * @param[in] cur_goal_idx  ==> index of the goal that we are trying to
     *                              connect to
     * @return  ==> whether or not the goal was connected to
     */
    bool connectToGoal(int cur_goal_idx);

    /**
     * Does the logistical work when one waypoint is reached from another
     *  - adds the node to the tree
     *  - keeps the dubins paths that got there, which outlive the tree
     *  - resets the tree with the goal as its new root
     *
     * @param[in] connection    ==> the connection to the goal to commit
     * @param[in] cur_goal_idx  ==> index of the goal that we are trying to connect to
     */
    void commitConnection(const Connection &connection, int cur_goal_idx);

    /**
     * The points flown along one leg, climbing from the altitude of the waypoint
     * behind it to the one it lands on
     *
     * @param[in] leg   ==> the leg to fly
     * @return  ==> the points along the leg, at altitude
     */
    std::vector<XYZCoord> buildFlightPath(const Leg &leg) const;
};

#endif  // INCLUDE_PATHING_RRT_HPP_
