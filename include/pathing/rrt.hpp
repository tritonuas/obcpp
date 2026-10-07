#ifndef INCLUDE_PATHING_RRT_HPP_
#define INCLUDE_PATHING_RRT_HPP_

#include <cmath>
#include <limits>
#include <utility>
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
 * One leg of the mission, flown from a waypoint to the one behind it.
 *
 * The tree that found the leg is thrown away as soon as the waypoint is reached,
 * so what it settled on is kept here instead. That is enough to say how long the
 * leg is, which is all a caller comparing two missions needs -- the points along
 * it are not worth flying until one of them has been picked.
 *
 * RRT is 2D and ignores altitude: the tree's points sit at z = 0, and the end
 * carries whatever altitude the goal was given. The altitudes a leg is flown
 * between are added in by PathGenerator.
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
 * hands back the best leg it could find to the waypoint it was asked for. A run
 * leaves the root where it found it, so a caller may price several goals from
 * the same place; advancing the mission is done by rerooting at the leg that
 * was picked. Stringing those legs together is not its concern -- see
 * PathGenerator.
 */
class RRT {
 public:
    // tree stores the nodes that form the tree
    RRTTree tree;

    // scratch space for bestConnection, which runs once for every sample RRT
    // takes -- nothing may read it between calls
    mutable std::vector<PathSegment> options;  // the paths out of the node being expanded

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
     * A run leaves the root where it found it, but it does leave the goal it
     * settled on hanging off the tree. Reroot before asking for the next
     * waypoint -- a run that follows one that is not rerooted may fly through
     * the goal of the run before it. @see RRT::reroot
     *
     * @param[in] goal      ==> the waypoint to path to
     * @param[in] angles    ==> the angles the goal may be approached at. A goal
     *                          that has to be flown at one particular heading is
     *                          a set of one.
     * @return  ==> the leg flown to the goal
     */
    Leg run(const XYZCoord &goal, const std::vector<double> &angles);

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
     *
     * @param[in] ends  ==> the points to path to
     * @return  ==> the node the flight hangs off of and the path out of it, or
     *              INVALID_NODE if nothing in the tree could reach any end
     */
    std::pair<NodeId, PathSegment> bestConnection(const std::vector<RRTPoint> &ends) const;

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
     * Hangs a connection to the goal off the tree and reads back the leg it flies
     *
     * The dubins paths are walked out of the tree here because they outlive it --
     * the tree they were found in is thrown away at the next reroot.
     *
     * @param[in] anchor    ==> the node the flight to the goal hangs off of
     * @param[in] segment   ==> the path flown from the anchor to the goal
     * @return  ==> the leg the connection flies
     */
    Leg commitConnection(NodeId anchor, const PathSegment &segment);

    /**
     * Throws away the tree and starts a new one rooted at the given waypoint
     *
     * This is how a mission advances: run() prices a leg without disturbing the
     * tree, and the caller reroots at the leg it settled on so the next one is
     * flown from where this one landed.
     *
     * @param[in] waypoint  ==> the waypoint the new tree is rooted at
     */
    void reroot(const RRTPoint &waypoint);
};

#endif  // INCLUDE_PATHING_RRT_HPP_
