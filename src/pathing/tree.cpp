#include "pathing/tree.hpp"

#include <algorithm>
#include <vector>

#include "pathing/dubins.hpp"
#include "utilities/datatypes.hpp"

RRTTree::RRTTree(const RRTPoint& root_point) { this->setCurrentHead(root_point); }

void RRTTree::addSample(NodeId parent_node, const PathSegment& new_segment) {
    const NodeId idx = size++;

    x[idx]      = new_segment.end.coord.x;
    y[idx]      = new_segment.end.coord.y;
    psi[idx]    = new_segment.end.psi;
    parent[idx] = parent_node;
    length[idx] = length[parent_node] + new_segment.path.length;
    path[idx]   = new_segment.path;
}

void RRTTree::setCurrentHead(const RRTPoint& goal) {
    size = 0;

    const NodeId idx = size++;

    x[idx]      = goal.coord.x;
    y[idx]      = goal.coord.y;
    psi[idx]    = goal.psi;
    parent[idx] = INVALID_NODE;
    length[idx] = 0.0;
    path[idx]   = DubinsPath(0.0, 0.0, 0.0, 0.0);
}

std::vector<PathSegment> RRTTree::findPathToNode(NodeId target_node) const {
    std::vector<PathSegment> segments;

    for (NodeId node = target_node;
         node != 0 && node != INVALID_NODE;
         node               = parent[node]) {
        segments.emplace_back(segment(node));
    }

    std::reverse(segments.begin(), segments.end());
    return segments;
}
