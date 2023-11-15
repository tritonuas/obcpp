#include "../../include/pathing/tree.hpp"

/* 
*  Hashes RRTPoint using the Cantor Pairing Function. 
*  Used to add elements to unordered_map nodeMap in RRTTree.
*/
std::size_t PointHashFunction::operator()(const RRTPoint &point) const {
    unsigned int h1 = std::hash<double>{}(point.xyz.x);
    unsigned int h2 = std::hash<double>{}(point.xyz.y);
    unsigned int h3 = std::hash<double>{}(point.xyz.z);

    unsigned int c1 = 0.5 * (h1 + h2) * (h1 + h2 + 1) + h2;
    unsigned int c2 = 0.5 * (c1 + h3) * (c1 + h3 + 1) + h3;

    return c2;
}

/* 
*  Hashes RRTNode* pair using the Cantor Pairing Function and RRTPoint hash function. 
*  Used to add elements to unordered_map edgeMap in RRTTree.
*/
std::size_t EdgeHashFunction::operator()(const std::pair<RRTNode*, RRTNode*> &nodePair) const{
    PointHashFunction p = PointHashFunction();
    unsigned int h1 = p(nodePair.first->getPoint());
    unsigned int h2 = p(nodePair.second->getPoint());

    unsigned int c1 = 0.5 * (h1 + h2) * (h1 + h2 + 1) + h2;
    
    return c1;
}

/*
* Constructor for RRTPoint object
*/
RRTPoint::RRTPoint(XYZCoord xyz, double psi)
    : xyz{xyz}, psi{psi} {}

/*
*  Equality overload method for RRTPoint
*/
bool RRTPoint::operator== (const RRTPoint &otherPoint) const {
    bool equal = this->xyz.x == otherPoint.xyz.x
                && this->xyz.y == otherPoint.xyz.y
                && this->xyz.z == otherPoint.xyz.z
                && this->psi == otherPoint.psi;

    return equal;
}

/*
*  Constructor for RRTNode object
*/
RRTNode::RRTNode(RRTPoint point, double cost)
    : point{point}, cost{cost} {}

/*
*  Overloaded constructor for RRTNode object
*/
RRTNode::RRTNode(RRTPoint point, double cost, RRTNodeList reachable)
    : point{point}, cost{cost}, reachable{reachable} {}

/*
*  Equality overload method for RRTNode object
*/
bool RRTNode::operator == (const RRTNode &otherNode) const {
    bool equal = this->point == otherNode.point
                && this->cost == otherNode.cost;

    return equal;
}

/*
*  Get the RRTPoint associated with this RRTNode object
*/
RRTPoint RRTNode::getPoint() {
    return this->point;
}

/*
*  Set the reachable (neighbors) list for this RRTNode object
*/
void RRTNode::setReachable(RRTNodeList reachable) {
    this->reachable = reachable;
    for(RRTNode* node : reachable) {
        node->parent = this;
    }
}

/*
*  Add a new node to the end of this node's reachable list.
*  Set the new node's parent to be this node.
*/
void RRTNode::addReachable(RRTNode* newNode) {
    this->reachable.push_back(newNode);
    newNode->parent = this;
}

/*
*  Remove a specific node from this node's reachable list.
*  Set the removed node's parent pointer to be null.
*/
void RRTNode::removeReachable(RRTNode* oldNode) {
    RRTNodeList* neighbors = this->getReachable();
    for(int i = 0; i < neighbors->size(); i++) {
        if(neighbors->at(i) == oldNode) {
            neighbors->at(i)->parent = nullptr;
            neighbors->erase(neighbors->begin() + i);
        }
    }
}

/*
*  Return a pointer to this node's reachable list
*/
RRTNodeList* RRTNode::getReachable() {
    return &(this->reachable);
}

/*
*  Get the cost associated with this node
*/
double RRTNode::getCost() {
    return this->cost;
}

/*
*  Set the cost associated with this node
*/
void RRTNode::setCost(double newCost) {
    this->cost = newCost;
}

/*
*  Constructor for RRTEdge object
*/
RRTEdge::RRTEdge(RRTNode* from, RRTNode* to, std::vector<RRTPoint> path, double cost)
    : from{from}, to{to}, path{path}, cost{cost} {}

/*
*  Equality overload method for RRTEdge comparison
*/
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

/*
*  Set the cost of this edge
*/
void RRTEdge::setCost(double newCost) {
    this->cost = newCost;
}

/*
*  Get the cost associated with this edge
*/
double RRTEdge::getCost() {
    return this->cost;
}

/*
*  Get the path vector associated with this edge
*/
std::vector<RRTPoint> RRTEdge::getPath() {
    return this->path;
}

/*
*  Set the path vector associated with this edge
*/
void RRTEdge::setPath(std::vector<RRTPoint> path) {
    this->path = path;
}

/*
*  Add a node to the RRTTree. 
*  If adding the first node to the tree, connectTo can be anything.
*/
void RRTTree::addNode(RRTNode* connectTo, RRTNode* newNode, std::vector<RRTPoint> path, double cost) {
    if(this->isEmpty) {
        std::pair<RRTPoint, RRTNode*> insertNode(newNode->getPoint(), newNode);
        nodeMap.insert(insertNode);
        this->isEmpty = false;
        return;
    }
    std::pair<RRTNode*, RRTNode*> edgePair(connectTo, newNode);
    RRTEdge newEdge = RRTEdge(connectTo, newNode, path, cost);
    
    edgeMap.insert(std::make_pair(edgePair, newEdge));
    std::pair<RRTPoint, RRTNode*> insertNode(newNode->getPoint(), newNode);
    nodeMap.insert(insertNode);

    connectTo->addReachable(newNode);
}

/*
* Delete an edge between 'from' and 'toPrev', and create a new edge 
* between 'from' to 'toNew'. Add 'toNew' to the nodeMap, and delete 'toPrev'.
*/
void RRTTree::rewireEdge(RRTNode* from, RRTNode* toPrev, RRTNode* toNew, std::vector<RRTPoint> path, double cost) {
    std::pair<RRTNode*, RRTNode*> toRemove(from, toPrev);
    std::pair<RRTNode*, RRTNode*> toAdd(from, toNew);
    
    RRTEdge newEdge = RRTEdge(from, toNew, path, cost);
    std::pair<std::pair<RRTNode*, RRTNode*>, RRTEdge> edgePair(toAdd, newEdge);

    // replace prev node in "from" node's neighbor list with new node 
    from->addReachable(toNew);
    from->removeReachable(toPrev);

    // remove old edge from edgeMap, add the new edge
    edgeMap.erase(toRemove);
    edgeMap.insert(edgePair);

    // remove old node from nodeMap, add the new node
    nodeMap.insert(std::make_pair(toNew->getPoint(), toNew));
    nodeMap.erase(toPrev->getPoint());
}

/*
*  Returns a pointer to the node in the tree corresponding to the RRTPoint.
*  If the node doesn't exist in the tree, returns nullptr.
*/
RRTNode* RRTTree::getNode(RRTPoint point) {
    if(nodeMap.count(point)) {
        return nodeMap.at(point);
    }
    else {
        return nullptr;
    }
}

/*
*  Returns a pointer to the edge in the tree corresponding to the point pair.
*  If the edge doesn't exist in the tree, returns nullptr.
*/
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