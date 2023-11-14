#include "../../include/pathing/tree.hpp"

// Hashes RRTPoint using the Cantor Pairing Function
std::size_t PointHashFunction::operator()(const RRTPoint &point) const {
    unsigned int h1 = std::hash<double>{}(point.xyz.x);
    unsigned int h2 = std::hash<double>{}(point.xyz.y);
    unsigned int h3 = std::hash<double>{}(point.xyz.z);

    unsigned int c1 = 0.5 * (h1 + h2) * (h1 + h2 + 1) + h2;
    unsigned int c2 = 0.5 * (c1 + h3) * (c1 + h3 + 1) + h3;

    return c2;
}

std::size_t EdgeHashFunction::operator()(const std::pair<RRTNode*, RRTNode*> &nodePair) const{
    PointHashFunction p = PointHashFunction();
    unsigned int h1 = p(nodePair.first->getPoint());
    unsigned int h2 = p(nodePair.second->getPoint());

    unsigned int c1 = 0.5 * (h1 + h2) * (h1 + h2 + 1) + h2;
    
    return c1;
}

RRTPoint::RRTPoint(XYZCoord xyz, double psi)
    : xyz{xyz}, psi{psi} {}

bool RRTPoint::operator== (const RRTPoint &otherPoint) const {
    bool equal = this->xyz.x == otherPoint.xyz.x
                && this->xyz.y == otherPoint.xyz.y
                && this->xyz.z == otherPoint.xyz.z
                && this->psi == otherPoint.psi;

    return equal;
}

RRTNode::RRTNode(RRTPoint point, double cost)
    : point{point}, cost{cost} {}

RRTNode::RRTNode(RRTPoint point, double cost, RRTNodeList reachable)
    : point{point}, cost{cost}, reachable{reachable} {}

bool RRTNode::operator == (const RRTNode &otherNode) const {
    bool equal = this->point == otherNode.point
                && this->cost == otherNode.cost;

    return equal;
}

RRTPoint RRTNode::getPoint() {
    return this->point;
}

void RRTNode::setReachable(RRTNodeList reachable) {
    this->reachable = reachable;
    for(RRTNode* node : reachable) {
        node->parent = this;
    }
}

void RRTNode::addReachable(RRTNode* newNode) {
    this->reachable.push_back(newNode);
    newNode->parent = this;
}

RRTNodeList* RRTNode::getReachable() {
    return &(this->reachable);
}

double RRTNode::getCost() {
    return this->cost;
}

void RRTNode::setCost(double newCost) {
    this->cost = newCost;
}

RRTEdge::RRTEdge(RRTNode* from, RRTNode* to, std::vector<RRTPoint> path, double cost)
    : from{from}, to{to}, path{path}, cost{cost} {}

bool RRTEdge::operator == (const RRTEdge &otherEdge) const {
    bool equal = this->from == otherEdge.from
                && this->to == otherEdge.to
                && this->cost == otherEdge.cost;
    if(otherEdge.path.size() != this->path.size()) {
        return false;
    }
    for(int i = 0; i < otherEdge.path.size(); i++) {
        if(!(this->path.at(i) == otherEdge.path.at(i))) {
            equal = false;
        }
    }
    return equal;
}

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
    
    edgeMap.insert(std::make_pair(edgePair, newEdge));
    std::pair<RRTPoint, RRTNode*> insertNode(newNode->getPoint(), newNode);
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
    RRTNodeList* neighbors = from->getReachable();
    for(int i = 0; i < neighbors->size(); i++) {
        if(neighbors->at(i) == toPrev) {
            neighbors->at(i) = toNew;
        }
    }

    // remove "from" node from prev node's neighbors
    neighbors = toPrev->getReachable();
    for (int i = 0; i < neighbors->size(); i++) {
        if(neighbors->at(i) == from) {
            neighbors->erase(neighbors->begin() + i);
        }
    }

    edgeMap.erase(toRemove);
    edgeMap.insert(edgePair);

    nodeMap.insert(std::make_pair(toNew->getPoint(), toNew));
    nodeMap.erase(toPrev->getPoint());
}

RRTNode* RRTTree::getNode(RRTPoint point) {
    if(nodeMap.count(point)) {
        return nodeMap.at(point);
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
    if(edgeMap.count(edgePair) > 0) {
        return &(edgeMap.at(edgePair));
    }
    else {
        return nullptr;
    }
}