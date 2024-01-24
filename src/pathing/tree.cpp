#include "pathing/tree.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/datatypes.hpp"

std::size_t EdgeHashFunction::operator()(const std::pair<RRTNode*, RRTNode*>& nodePair) const {
    PointHashFunction p = PointHashFunction();
    unsigned int h1 = p(nodePair.first->getPoint());
    unsigned int h2 = p(nodePair.second->getPoint());

    unsigned int c1 = 0.5 * (h1 + h2) * (h1 + h2 + 1) + h2;

    return c1;
}

RRTNode::RRTNode(RRTPoint point, double cost) : point{point}, cost{cost} {}

RRTNode::RRTNode(RRTPoint point, double cost, RRTNodeList reachable)
    : point{point}, cost{cost}, reachable{reachable} {}

bool RRTNode::operator==(const RRTNode& otherNode) const {
    return (this->point == otherNode.point && this->cost == otherNode.cost);
}

RRTPoint RRTNode::getPoint() { return this->point; }

void RRTNode::setReachable(RRTNodeList reachable) {
    this->reachable = reachable;
    for (RRTNode* node : reachable) {
        node->parent = this;
    }
}

void RRTNode::addReachable(RRTNode* newNode) {
    this->reachable.push_back(newNode);
    newNode->parent = this;
}

void RRTNode::removeReachable(RRTNode* oldNode) {
    RRTNodeList neighbors = this->getReachable();
    for (int i = 0; i < neighbors.size(); i++) {
        if (neighbors.at(i) == oldNode) {
            neighbors.at(i)->parent = nullptr;
            neighbors.erase(neighbors.begin() + i);
        }
    }
}

const RRTNodeList& RRTNode::getReachable() { return (this->reachable); }

double RRTNode::getCost() const { return this->cost; }

void RRTNode::setCost(double newCost) { this->cost = newCost; }

RRTEdge::RRTEdge(RRTNode* from, RRTNode* to, std::vector<XYZCoord> path, double cost)
    : from{from}, to{to}, path{path}, cost{cost} {}

bool RRTEdge::operator==(const RRTEdge& otherEdge) const {
    bool equal =
        this->from == otherEdge.from && this->to == otherEdge.to && this->cost == otherEdge.cost;
    if (!equal || otherEdge.path.size() != this->path.size()) {
        return false;
    }
    for (int i = 0; i < otherEdge.path.size(); i++) {
        if (!(this->path.at(i) == otherEdge.path.at(i))) {
            return false;
        }
    }
    return true;
}

void RRTEdge::setCost(double newCost) { this->cost = newCost; }

double RRTEdge::getCost() const { return this->cost; }

const std::vector<XYZCoord>& RRTEdge::getPath() { return this->path; }

void RRTEdge::setPath(std::vector<XYZCoord> path) { this->path = path; }

/** RRTTree */
RRTTree::RRTTree(RRTPoint rootPoint, Environment airspace, Dubins dubins)
    : airspace{airspace}, dubins{dubins} {
    RRTNode* newNode = new RRTNode(rootPoint, LARGE_COST);
    nodeMap.insert(std::make_pair(rootPoint, newNode));
}

// TODO - seems a bit sketchy
RRTTree::~RRTTree() {
    for (std::pair<RRTPoint, RRTNode*> node : nodeMap) {
        delete node.second;
    }
}

// TODO - convert from old to new
bool RRTTree::addNode(RRTNode* connectTo, RRTPoint newPoint) {
    RRTNode* newNode = new RRTNode(newPoint, LARGE_COST);

    if (this->nodeMap.empty()) {
        std::pair<RRTPoint, RRTNode*> insertNode(newNode->getPoint(), newNode);
        nodeMap.insert(insertNode);
        return true;
    }

    // checking if path is valid
    const std::vector<RRTOption> dubins_options =
        dubins.allOptions(connectTo->getPoint(), newNode->getPoint(), true);
    double cost = LARGE_COST;
    std::vector<XYZCoord> path{};

    // find the best valid path
    for (const RRTOption& option : dubins_options) {
        if (option.length == std::numeric_limits<double>::infinity()) {
            return false;
        }

        // generate the path
        std::vector<XYZCoord> current_path = dubins.generatePoints(
            connectTo->getPoint(), newNode->getPoint(), option.dubins_path, option.has_straight);

        // check if given path is inbounds
        if (airspace.isPathInBounds(current_path)) {
            if (option.length < cost) {
                cost = option.length;
                path = current_path;
                break;
            }
        }
    }

    if (cost == LARGE_COST) {
        std::cout << "PANIC PANIC TREE.CPP:131" << std::endl;
        return false;
    }

    std::pair<RRTNode*, RRTNode*> edgePair(connectTo, newNode);
    RRTEdge newEdge = RRTEdge(connectTo, newNode, path, cost);

    edgeMap.insert(std::make_pair(edgePair, newEdge));
    std::pair<RRTPoint, RRTNode*> insertNode(newNode->getPoint(), newNode);
    nodeMap.insert(insertNode);

    connectTo->addReachable(newNode);

    return true;
}

void RRTTree::rewireEdge(RRTNode* from, RRTNode* toPrev, RRTNode* toNew, std::vector<XYZCoord> path,
                         double cost) {
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

RRTNode* RRTTree::getNode(RRTPoint point) {
    if (nodeMap.count(point)) {
        return nodeMap.at(point);
    } else {
        return nullptr;
    }
}

RRTEdge* RRTTree::getEdge(RRTPoint from, RRTPoint to) {
    RRTNode* node1 = getNode(from);
    RRTNode* node2 = getNode(to);
    if (node1 == nullptr || node2 == nullptr) {
        return nullptr;
    }

    std::pair<RRTNode*, RRTNode*> edgePair(node1, node2);
    if (edgeMap.count(edgePair) > 0) {
        return &(edgeMap.at(edgePair));
    } else {
        return nullptr;
    }
}
