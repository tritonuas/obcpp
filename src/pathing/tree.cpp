#include "pathing/tree.hpp"

#include <algorithm>
#include <vector>

#include "pathing/dubins.hpp"
#include "utilities/datatypes.hpp"

RRTTree::RRTTree(RRTPoint root_point) { this->setCurrentHead(root_point); }

void RRTTree::addSample(NodeId parent, const RRTPoint new_point, const RRTOption option) {
    NodeId prev_sibling = tree.first_child[parent];
    NodeId idx = tree.alloc();

    tree.points[idx]       = new_point;
    tree.rrt_options[idx]  = option;
    tree.parent[idx]       = parent;
    tree.length[idx]       = tree.length[parent] + option.length;
    tree.first_child[idx]  = INVALID_NODE;
    tree.next_sibling[idx] = INVALID_NODE;

    if (prev_sibling == INVALID_NODE) {
        tree.first_child[parent] = idx;
    } else {
        while (tree.next_sibling[prev_sibling] != INVALID_NODE) {
            prev_sibling = tree.next_sibling[prev_sibling];
        }
        tree.next_sibling[prev_sibling] = idx;
    }
}

void RRTTree::setCurrentHead(RRTPoint goal) {
    tree.reset();

    NodeId idx = tree.alloc();

    tree.points[idx]       = goal;
    tree.rrt_options[idx]  = RRTOption(0.0, DubinsPath(0.0, 0.0, 0.0), true);
    tree.parent[idx]       = INVALID_NODE;
    tree.length[idx]       = 0.0;
    tree.first_child[idx]  = INVALID_NODE;
    tree.next_sibling[idx] = INVALID_NODE;
}

RRTPoint RRTTree::getStart() const { return tree.points[0]; }

std::vector<PathSegment> RRTTree::findPathToNode(NodeId target_node) const {
    std::vector<PathSegment> segments;

    for (NodeId node = target_node;
         node != 0 && node != INVALID_NODE;
         node = tree.parent[node]) {
        segments.emplace_back(tree.points[node], tree.rrt_options[node]);
    }

    std::reverse(segments.begin(), segments.end());
    return segments;
}
