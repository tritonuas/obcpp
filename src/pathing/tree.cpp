#include "pathing/tree.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/datatypes.hpp"

std::size_t EdgeHashFunction::operator()(const std::pair<RRTNode*, RRTNode*>& node_pair) const {
    PointHashFunction p = PointHashFunction();
    unsigned int h1 = p(node_pair.first->getPoint());
    unsigned int h2 = p(node_pair.second->getPoint());

    unsigned int c1 = 0.5 * (h1 + h2) * (h1 + h2 + 1) + h2;

    return c1;
}

RRTNode::RRTNode(RRTPoint point, double cost) : point{point}, cost{cost} {}

RRTNode::RRTNode(RRTPoint point, double cost, RRTNodeList reachable)
    : point{point}, cost{cost}, reachable{reachable} {}

bool RRTNode::operator==(const RRTNode& other_node) const {
    return this->point == other_node.point && this->cost == other_node.cost;
}

RRTPoint RRTNode::getPoint() { return this->point; }

void RRTNode::setReachable(RRTNodeList reachable) {
    this->reachable = reachable;
    for (RRTNode* node : reachable) {
        node->parent = this;
    }
}

void RRTNode::addReachable(RRTNode* new_node) {
    this->reachable.push_back(new_node);
    new_node->parent = this;
}

void RRTNode::removeReachable(RRTNode* old_node) {
    for (int i = 0; i < reachable.size(); i++) {
        if (reachable.at(i) == old_node) {
            reachable.at(i)->parent = nullptr;
            reachable.erase(reachable.begin() + i);
        }
    }
}

const RRTNodeList& RRTNode::getReachable() { return (this->reachable); }

double RRTNode::getCost() const { return this->cost; }

void RRTNode::setCost(double new_cost) { this->cost = new_cost; }

RRTNode* RRTNode::getParent() const { return this->parent; }

RRTEdge::RRTEdge(RRTNode* from, RRTNode* to, std::vector<XYZCoord> path, double cost)
    : from{from}, to{to}, path{path}, cost{cost} {}

bool RRTEdge::operator==(const RRTEdge& other_edge) const {
    bool equal =
        this->from == other_edge.from && this->to == other_edge.to && this->cost == other_edge.cost;
    if (!equal || other_edge.path.size() != this->path.size()) {
        return false;
    }
    for (int i = 0; i < other_edge.path.size(); i++) {
        if (!(this->path.at(i) == other_edge.path.at(i))) {
            return false;
        }
    }
    return true;
}

void RRTEdge::setCost(double new_cost) { this->cost = new_cost; }

double RRTEdge::getCost() const { return this->cost; }

const std::vector<XYZCoord>& RRTEdge::getPath() { return this->path; }

void RRTEdge::setPath(std::vector<XYZCoord> path) { this->path = path; }

/** RRTTree */
RRTTree::RRTTree(RRTPoint root_point, Environment airspace, Dubins dubins)
    : airspace{airspace}, dubins{dubins} {
    RRTNode* newNode = new RRTNode(root_point, LARGE_COST);
    root = newNode;
    node_map.insert(std::make_pair(root_point, newNode));
}

// TODO - seems a bit sketchy
RRTTree::~RRTTree() {
    for (std::pair<RRTPoint, RRTNode*> node : node_map) {
        delete node.second;
    }
}

// TODO - convert from old to new
bool RRTTree::addNode(RRTNode* anchor_node, RRTPoint& new_point, const RRTOption& option) {
    // checking if path is valid

    std::vector<XYZCoord> path = dubins.generatePoints(anchor_node->getPoint(), new_point,
                                                       option.dubins_path, option.has_straight);

    if (!airspace.isPathInBounds(path)) {
        return false;
    }
    // if a valid path was found, it will add the node to the tree

    RRTNode* new_node = new RRTNode(new_point, LARGE_COST);

    std::pair<RRTNode*, RRTNode*> edgePair(anchor_node, new_node);
    RRTEdge new_edge = RRTEdge(anchor_node, new_node, path, option.length);
    edge_map.insert(std::make_pair(edgePair, new_edge));

    std::pair<RRTPoint, RRTNode*> insertNode(new_node->getPoint(), new_node);
    node_map.insert(insertNode);

    anchor_node->addReachable(new_node);
    if (airspace.isPointInGoal(new_point.coord)) {
        airspace.setGoalfound();
        retreivePathByNode(new_node, new_node->getParent());
    }

    return true;
}

void RRTTree::rewireEdge(RRTNode* current_node, RRTNode* previous_connection,
                         RRTNode* new_connection, std::vector<XYZCoord> path, double cost) {
    // if the new_node doesn't have a parent, then this code will crash
    std::pair<RRTNode*, RRTNode*> previous_pair(current_node, previous_connection);
    std::pair<RRTNode*, RRTNode*> new_pair(current_node, new_connection);

    RRTEdge new_edge = RRTEdge(current_node, new_connection, path, cost);
    std::pair<std::pair<RRTNode*, RRTNode*>, RRTEdge> edgePair(new_pair, new_edge);
    // replace prev node in "from" node's neighbor list with new node

    // based on testing, if current_point and new_connection are already connected
    // there will be duplicate edges in the edge_map
    current_node->addReachable(new_connection);
    current_node->removeReachable(previous_connection);

    // remove old edge from edge_map, add the new edge
    edge_map.erase(previous_pair);
    edge_map.insert(edgePair);

    // remove old node from node_map, add the new node
    node_map.insert(std::make_pair(new_connection->getPoint(), new_connection));
    node_map.erase(previous_connection->getPoint());
}

RRTNode* RRTTree::getNode(RRTPoint point) {
    if (node_map.count(point)) {
        return node_map.at(point);
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
    if (edge_map.count(edgePair) > 0) {
        return &(edge_map.at(edgePair));
    } else {
        return nullptr;
    }
}

RRTNode* RRTTree::getRoot() const { return this->root; }

RRTPoint RRTTree::getGoal() const { return this->airspace.getGoal(); }

Environment RRTTree::getAirspace() const { return this->airspace; }

RRTPoint RRTTree::getRandomPoint(double search_radius) const {
    return airspace.getRandomPoint(root->getPoint(), search_radius);
}

void RRTTree::retreivePathByNode(RRTNode* node, RRTNode* parent) {
    if (parent == nullptr || node == nullptr) {
        return;
    }

    retreivePathByNode(parent, parent->getParent());

    std::vector<XYZCoord> edge = getEdge(parent->getPoint(), node->getPoint())->getPath();

    path_to_goal.insert(path_to_goal.end(), edge.begin(), edge.end());
}
