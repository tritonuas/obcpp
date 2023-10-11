#include "tree.hpp"

RRTPoint::RRTPoint(double x, double y, double z, double psi)
    : x{x}, y{y}, z{z}, psi{psi} {}


RRTNode::RRTNode(RRTPoint point)
    : point{point} {}

RRTNode::RRTNode(RRTPoint point, RRTNodeList reachable)
    : point{point}, reachable{reachable} {}

RRTPoint RRTNode::getPoint() {
    return point;
}