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

template <uint32_t N>
struct TreeArray {
    static_assert(N < INVALID_NODE, "TREE is TOO LARGE");

    std::array<RRTPoint,  N> points;
    std::array<RRTOption, N> rrt_options;
    std::array<double,    N> length;
    std::array<NodeId,    N> parent;
    std::array<NodeId,    N> first_child;   // INVALID_NODE if leaf
    std::array<NodeId,    N> next_sibling;  // INVALID_NODE if last
    NodeId size = 0;

    NodeId alloc() { return size++; }
    void   reset() { size = 0; }
};

class RRTTree {
 public:
    TreeArray<TREE_CAPACITY> tree;

    explicit RRTTree(RRTPoint root_point);

    /**
     *  Add a node to the RRTTree. ASSUMES ROOT EXISTS
     *
     * @param[in] parent    ==> The parent of the new node
     * @param[in] new_point ==> RRTPoint for the new node
     * @param[in] option    ==> RRTOption for the new node
     */
    void addSample(NodeId parent, const RRTPoint new_point, const RRTOption option);

    /**
     * Changes the currentHead to the given goal
     *
     * @param[in] goal  ==> the goal to change the currentHead to
     */
    void setCurrentHead(RRTPoint goal);

    /**
     * Returns the start RRTPoint
     *
     * @return RRTPoint start point
     */
    RRTPoint getStart() const;

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
