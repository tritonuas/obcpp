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
    RRTPoint start{};                   // the waypoint the leg is flown from
    RRTPoint end{};                     // the waypoint the leg lands on
    std::vector<PathSegment> segments;  // the dubins paths flown, in order
    double length = 0;                  // the ground the leg covers

    inline bool isValid() const { return !segments.empty(); }
};

/**
 * The points a goal can be reached at, one for every angle it may be approached
 * at -- which is a single point when the caller pinned it down
 *
 * @param[in] goal      ==> the waypoint being pathed to
 * @param[in] angles    ==> the angles it may be approached at
 * @return  ==> the goal, at each of the approach angles
 */
std::vector<RRTPoint> goalEndpoints(const XYZCoord &goal, const std::vector<double> &angles);

/**
 * Searches out the flying between one waypoint and the next.
 *
 * The tree is rooted at the waypoint the plane is flying from, and each run
 * hands back the best leg it could find to the waypoint it was asked for, then
 * re-roots itself there so the leg after it starts where this one landed.
 * Stringing those legs into a mission is not its concern -- see PathGenerator.
 */
class RRT {
 public:
    // tree stores the nodes that form the tree
    RRTTree tree;

    /*
     * Scratch space for bestConnection, which runs once for every sample RRT takes
     * and would otherwise lay all of this out again each time. It is not state --
     * nothing may read it between calls, and every call fills it before it reads it.
     */
    mutable std::array<double, TREE_CAPACITY> bounds;    // by node, the cheapest flight through it
    mutable std::array<NodeId, TREE_CAPACITY> frontier;  // the order the nodes are looked at in
    mutable std::vector<Connection> options;             // the paths out of the node being expanded

    /**
     * @param[in] start ==> the vector the plane is flying, which the tree is
     *                      rooted at
     */
    explicit RRT(RRTPoint start);

    /**
     * The best leg RRT could come up with between where the tree is rooted and
     * the given goal
     *
     * The direct flight is tried first, as nothing sampling could turn up beats
     * it. Only when the goal cannot be reached in one path is a tree grown.
     *
     * @param[in] goal      ==> the waypoint to path to
     * @param[in] angles    ==> the angles the goal may be approached at. A goal
     *                          that has to be flown at one particular heading is
     *                          a set of one.
     * @return  ==> the leg flown to the goal
     */
    Leg run(const XYZCoord &goal, const std::vector<double> &angles);

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
     * @param[in] ends  ==> the points to path to
     * @return  ==> the connection if one was found, an invalid connection
     *              otherwise
     */
    Connection bestConnection(const std::vector<RRTPoint> &ends) const;

    /**
     * Does a single iteration of the RRT(star) algoritm to connect two waypoints
     *
     * @param[in] goal  ==> the waypoint being pathed to, which the sampling is
     *                      biased towards
     * @param[in] ends  ==> the goal, at each angle it may be approached at
     * @return  ==> the leg flown to the goal
     */
    Leg RRTIteration(const XYZCoord &goal, const std::vector<RRTPoint> &ends);

    /**
     * Connects to the goal after RRT is finished
     *
     * @param[in] ends  ==> the goal, at each angle it may be approached at
     * @return  ==> the leg flown to the goal, an invalid leg if it could not be
     *              reached
     */
    Leg connectToGoal(const std::vector<RRTPoint> &ends);

    /**
     * Does the logistical work when one waypoint is reached from another
     *  - adds the node to the tree
     *  - keeps the dubins paths that got there, which outlive the tree
     *  - resets the tree with the goal as its new root
     *
     * @param[in] connection    ==> the connection to the goal to commit
     * @return  ==> the leg the connection flies
     */
    Leg commitConnection(const Connection &connection);
};

#endif  // INCLUDE_PATHING_RRT_HPP_
