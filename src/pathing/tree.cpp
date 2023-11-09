#include "../../include/pathing/tree.hpp"

RRTPoint::RRTPoint(XYZCoord xyz, double psi)
    : xyz{xyz}, psi{psi} {}


RRTNode::RRTNode(RRTPoint point, double cost)
    : point{point}, cost{cost} {}

RRTNode::RRTNode(RRTPoint point, double cost, RRTNodeList reachable)
    : point{point}, cost{cost}, reachable{reachable} {}

RRTPoint RRTNode::getPoint() {
    return this->point;
}

void RRTNode::setReachable(RRTNodeList reachable) {
    this->reachable = reachable;
}

void RRTNode::addReachable(RRTNode* newNode) {
    this->reachable.push_back(newNode);
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

RRTEdge::RRTEdge(RRTNode* from, RRTNode* to, std::vector<RRTPoint> path, double cost)
    : from{from}, to{to}, path{path}, cost{cost} {}

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

void RRTTree::addNode(RRTNode* connectTo, RRTNode* newNode, std::vector<RRTPoint> path, double cost) {
    std::pair<RRTNode*, RRTNode*> edgePair(connectTo, newNode);
    RRTEdge newEdge = RRTEdge(connectTo, newNode, path, cost);
    std::pair<std::pair<RRTNode*, RRTNode*>, RRTEdge> toAdd(edgePair, newEdge);
    
    edgeMap.insert(toAdd);
    std::pair<RRTPoint, RRTNode> insertNode(newNode->getPoint(), *newNode);
    nodeMap.insert(insertNode);

    connectTo->addReachable(newNode);
    newNode->addReachable(connectTo);
}

void RRTTree::rewireEdge(RRTNode* from, RRTNode* toPrev, RRTNode* toNew, std::vector<RRTPoint> path, double cost) {
    std::pair<RRTNode*, RRTNode*> toRemove(from, toPrev);
    std::pair<RRTNode*, RRTNode*> toAdd(from, toNew);
    
    RRTEdge newEdge = RRTEdge(from, toNew, path, cost);
    std::pair<std::pair<RRTNode*, RRTNode*>, RRTEdge> edgePair(toAdd, newEdge);

    // replace prev node in "from" node's neighbor list with new node 
    RRTNodeList neighbors = from->getReachable();
    for(int i = 0; i < neighbors.size(); i++) {
        if(neighbors.at(i) == toPrev) {
            neighbors[i] = toNew;
        }
    }
    // remove "from" node from prev node's neighbors
    neighbors = toPrev->getReachable();
    for (RRTNodeList::iterator it = neighbors.begin() ; it != neighbors.end(); ++it) {
        if(*it == from) {
            neighbors.erase(it);
        }
    }
    edgeMap.erase(toRemove);
    edgeMap.insert(edgePair);
}

RRTNode* RRTTree::getNode(RRTPoint point) {
    if(nodeMap.count(point)) {
        return &(nodeMap.at(point));
    }
    else {
        return nullptr;
    }
}

RRTEdge* RRTTree::getEdge(RRTPoint from, RRTPoint to) {
    RRTNode* node1 = getNode(from);
    RRTNode* node2 = getNode(to);
    if(node1 == nullptr || node2 == nullptr) {
        return nullptr;
    }
    std::pair<RRTNode*, RRTNode*> edgePair(node1, node2);
    if(edgeMap.count(edgePair)) {
        return &(edgeMap.at(edgePair));
    }
    else {
        return nullptr;
    }
}