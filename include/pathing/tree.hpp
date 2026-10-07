#ifndef INCLUDE_PATHING_TREE_HPP_
#define INCLUDE_PATHING_TREE_HPP_

#include <array>
#include <cstdint>
#include <vector>

#include "pathing/dubins.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

using NodeId = uint16_t;
constexpr NodeId INVALID_NODE = 0xFFFF;  // HARDCODED FOR uint16_t

static_assert(TREE_CAPACITY < INVALID_NODE, "TREE is TOO LARGE");

/**
 * The nodes RRT has connected up, stored a field at a time.
 *
 * The search looks at every node in the tree on every sample it takes, so the
 * three fields it reads the most are laid out as their own runs of doubles.
 */
class RRTTree {
 public:
    // read by the search on every sample -- keep flat and contiguous
    std::array<double, TREE_CAPACITY> x;       // where the node sits
    std::array<double, TREE_CAPACITY> y;
    std::array<double, TREE_CAPACITY> length;  // ground covered flying from the root to here

    // only read once a node has been picked
    std::array<double,     TREE_CAPACITY> psi;     // the heading the node is flown at
    std::array<DubinsPath, TREE_CAPACITY> path;    // the path flown in from the parent
    std::array<NodeId,     TREE_CAPACITY> parent;  // INVALID_NODE for the root

    NodeId size = 0;

    explicit RRTTree(const RRTPoint& root_point);

    /**
     * The vector a node sits on.
     *
     * The tree is 2D -- a dubins path is a flat curve, so no node carries an
     * altitude, the root included. Altitude is added once the leg has been
     * settled on. @see PathGenerator::buildFlightPath
     */
    RRTPoint point(NodeId node) const { return RRTPoint(XYZCoord{x[node], y[node], 0}, psi[node]); }

    /**
     * The path a node was reached by, and the vector it lands on.
     *
     * @param[in] node  ==> the node to describe, which may not be the root
     */
    PathSegment segment(NodeId node) const { return PathSegment(point(node), path[node]); }

    /**
     *  Add a node to the RRTTree. ASSUMES ROOT EXISTS
     *
     * @param[in] parent_node   ==> The parent of the new node
     * @param[in] new_segment  ==> the path flown from the parent, and where it lands
     */
    void addSample(NodeId parent_node, const PathSegment& new_segment);

    /**
     * Changes the currentHead to the given goal
     *
     * @param[in] goal  ==> the goal to change the currentHead to
     */
    void setCurrentHead(const RRTPoint& goal);

    /**
     * Returns the start RRTPoint, which is the root (node 0)
     *
     * @return RRTPoint start point
     */
    RRTPoint getStart() const { return point(0); }

    /**
     * Finds the sequence of dubins paths flown from current_head to the target node
     *
     * Every node stores the path its parent takes to reach it, and knows its
     * parent, so the path is walked up to the root instead of searched down from
     * it. The root itself contributes nothing, as it is where the path starts.
     *
     * A node's point is where the path stored on it lands, so the two travel
     * together and the vectors never have to be recomputed downstream.
     *
     * @param[in] target_node   ==> the node to find the path to
     * @return  ==> segments from current_head to target_node, in flight order
     */
    std::vector<PathSegment> findPathToNode(NodeId target_node) const;
};

#endif  // INCLUDE_PATHING_TREE_HPP_
