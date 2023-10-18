#include "tree.hpp"

RRTPoint::RRTPoint(double x, double y, double z, double psi)
    : x{x}, y{y}, z{z}, psi{psi} {}


RRTNode::RRTNode(RRTPoint point)
    : point{point} {}

RRTNode::RRTNode(RRTPoint point, RRTNodeList reachable)
    : point{point}, reachable{reachable} {}

RRTPoint RRTNode::getPoint() {
    return this->point;
}

void RRTNode::setReachable(RRTNodeList reachable) {
    this->reachable = reachable;
}

RRTNodeList RRTNode::getReachable() {
    return this->reachable;
}

double RRTNode::getCost() {
    return this->cost;
}

void RRTNode::setCost(double newCost) {
    this->cost = newCost;
}

RRTEdge::RRTEdge(RRTNode* from, RRTNode* to) 
    : from{from}, to{to} {}

RRTEdge::RRTEdge(RRTNode* from, RRTNode* to, std::vector<RRTPoint> path)
    : from{from}, to{to} {}

void RRTEdge::setCost(double newCost) {
    this->cost = newCost;
}

double RRTEdge::getCost() {
    return this->cost;
}

std::vector<RRTPoint> RRTEdge::getPath() {
    return this->path;
}

void RRTEdge::setPath(std::vector<RRTPoint> path) {
    this->path = path;
}

void RRTTree::addNode(RRTNode* connectTo) {

}

void RRTTree::rewireEdge(RRTNode* from, RRTNode* toPrev, RRTNode* toNew) {

}

RRTNode* RRTTree::getNode(RRTPoint point) {
    
}

RRTEdge RRTTree::getEdge(RRTPoint from, RRTPoint to) {

}