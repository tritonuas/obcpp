#include "pathing/tree.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

std::size_t EdgeHashFunction::operator()(const std::pair<RRTNode*, RRTNode*>& node_pair) const {
    PointHashFunction p = PointHashFunction();
    unsigned int h1 = p(node_pair.first->getPoint());
    unsigned int h2 = p(node_pair.second->getPoint());

    unsigned int c1 = 0.5 * (h1 + h2) * (h1 + h2 + 1) + h2;

    return c1;
}

RRTNode::RRTNode(const RRTPoint& point, double cost) : point{point}, cost{cost} {}

RRTNode::RRTNode(const RRTPoint& point, double cost, RRTNodeList& reachable)
    : point{point}, cost{cost}, reachable{reachable} {}

bool RRTNode::operator==(const RRTNode& other_node) const {
    return this->point == other_node.point && this->cost == other_node.cost;
}

RRTPoint RRTNode::getPoint() { return this->point; }

void RRTNode::setReachable(const RRTNodeList& reachable) {
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
            // TODO - UNSAFE
            reachable.at(i)->parent = nullptr;
            reachable.erase(reachable.begin() + i);
        }
    }
}

const RRTNodeList& RRTNode::getReachable() { return (this->reachable); }

double RRTNode::getCost() const { return this->cost; }

void RRTNode::setCost(double new_cost) { this->cost = new_cost; }

RRTNode* RRTNode::getParent() const { return this->parent; }

void RRTNode::setParent(RRTNode* new_parent) { this->parent = new_parent; }
/** RRTEdge */

RRTEdge::RRTEdge(RRTNode* from, RRTNode* to, const std::vector<XYZCoord>& path, double cost)
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
    RRTNode* new_node = new RRTNode(root_point, LARGE_COST);
    root = new_node;
    current_head = new_node;
    node_map.insert({root_point, new_node});
    root->setCost(0);
}

// TODO - seems a bit sketchy
RRTTree::~RRTTree() {
    for (std::pair<RRTPoint, RRTNode*> node : node_map) {
        delete node.second;
    }
}

// TODO - convert from old to new
RRTNode* RRTTree::addNode(RRTNode* anchor_node, const RRTPoint& new_point,
                          const RRTOption& option) {
    // checking if path is valid

    // debugging text
    // std::cout << "Anchor: " << anchor_node << std::endl;
    // std::cout << "Anchor: (" << anchor_node->getPoint().coord.x << ", "
    //           << anchor_node->getPoint().coord.y << ", " << anchor_node->getPoint().coord.z <<
    //           ")"
    //           << "psi: " << anchor_node->getPoint().psi << std::endl;
    // std::cout << "New: (" << new_point.coord.x << ", " << new_point.coord.y << ", "
    //           << new_point.coord.z << ")"
    //           << "psi: " << new_point.psi << std::endl;
    // std::cout << "Option length: " << option.length << std::endl;

    std::vector<XYZCoord> path = dubins.generatePoints(anchor_node->getPoint(), new_point,
                                                       option.dubins_path, option.has_straight);

    if (!airspace.isPathInBounds(path)) {
        return nullptr;
    }

    // if a valid path was found, it will add the node to the tree
    RRTNode* new_node = new RRTNode(new_point, anchor_node->getCost() + option.length);

    // creating new edge
    std::pair<RRTNode*, RRTNode*> edgePair(anchor_node, new_node);
    RRTEdge new_edge = RRTEdge(anchor_node, new_node, path, option.length);
    edge_map.emplace(edgePair, new_edge);

    // inserting the node into the tree
    node_map.emplace(new_node->getPoint(), new_node);

    // changing state of its parent
    anchor_node->addReachable(new_node);

    return new_node;
}

void RRTTree::rewireEdge(RRTNode* current_node, RRTNode* previous_parent, RRTNode* new_parent,
                         const std::vector<XYZCoord>& path, double cost) {
    // if the new_node doesn't have a parent, then this code will crash
    std::pair<RRTNode*, RRTNode*> previous_pair(previous_parent, current_node);
    std::pair<RRTNode*, RRTNode*> new_pair(new_parent, current_node);

    RRTEdge new_edge = RRTEdge(new_parent, current_node, path, cost);
    // replace prev node in "from" node's neighbor list with new node

    // ORDER MATTERS, REMOVE THEN ADD TO PRESERVE THE CURR_NODE HAS A PARENT
    previous_parent->removeReachable(current_node);
    new_parent->addReachable(current_node);

    // remove old edge from edge_map, add the new edge
    edge_map.erase(previous_pair);
    edge_map.emplace(new_pair, new_edge);
    current_node->setCost(new_parent->getCost() + cost);

    reassignCosts(current_node);
}

// ALSO BROKEN
RRTNode* RRTTree::getNode(const RRTPoint& point) {
    if (node_map.count(point) > 0) {
        return node_map.at(point);
    } else {
        return nullptr;
    }
}

// BROKEN
RRTEdge* RRTTree::getEdge(const RRTPoint& from, const RRTPoint& to) {
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

XYZCoord RRTTree::getGoal() const { return airspace.getGoal(); }

XYZCoord RRTTree::getGoal(int index) const { return airspace.getGoal(index); }

Environment RRTTree::getAirspace() const { return this->airspace; }

RRTPoint RRTTree::getRandomPoint(double search_radius) {
    // gets random point if the goal is not being used
    const RRTPoint& sample = airspace.getRandomPoint();

    // // picks the nearest node to the sample, and then returns a point `search_radius` distance
    // away
    // // from tat point in the direction of the sample
    // RRTNode* nearest_node = getNearestNode(sample);

    // // shouldn't happen, it is here for memory safety
    // if (nearest_node == nullptr) {
    //     return sample;
    // }

    // const RRTPoint& nearest_point = nearest_node->getPoint();

    // const XYZCoord displacement_vector = sample.coord - nearest_point.coord;

    // // TODO - use some heuristic to make this angle as optimal as possible
    // const double angle =
    //     std::atan2(displacement_vector.y, displacement_vector.x) + random(-M_PI / 2, M_PI / 2);

    // // distance between the vectors, if it is less than the search radius, then it will return
    // the
    // // sample point, otherwise, it will return a point `search_radius` away from the nearest
    // point const double distance = displacement_vector.norm();

    // if (distance < search_radius) {
    //     return RRTPoint(sample.coord, angle);
    // }

    // RRTPoint new_point(nearest_point.coord + (search_radius / distance) * displacement_vector,
    //                    angle);

    // return new_point;
    return RRTPoint(sample.coord, random(0, TWO_PI));
}

std::vector<std::pair<RRTNode*, RRTOption>> RRTTree::pathingOptions(const RRTPoint& end,
                                                                    int quantity_options) {
    // fills the options list with valid values
    std::vector<std::pair<RRTNode*, RRTOption>> options;
    fillOptions(&options, current_head, end);

    // sorts the list
    std::sort(options.begin(), options.end(), [](auto a, auto b) {
        auto [a_node, a_option] = a;
        auto [b_node, b_option] = b;
        return a_option.length + a_node->getCost() < b_option.length + b_node->getCost();
    });

    // the options are already sorted, why return a truncated list?

    // if there are less options than needed amount, then just reurn the xisting list, else,
    // return a truncated list.
    // if (options.size() < quantity_options) {
    //     return options;
    // }

    // erases everything after the last wanted element
    // options.erase(options.begin() + quantity_options, options.end());

    return options;
}

void RRTTree::fillOptions(std::vector<std::pair<RRTNode*, RRTOption>>* options, RRTNode* node,
                          const RRTPoint& end) {
    /*
       TODO - try to limit the scope of the search to prevent too many calls to dubins
    */
    if (node == nullptr) {
        return;
    }

    // gets all dubins curves from the current node to the end point
    std::vector<RRTOption> local_options = dubins.allOptions(node->getPoint(), end);

    // filters out the options that are not valid
    for (const RRTOption& option : local_options) {
        if (std::isnan(option.length) || option.length == std::numeric_limits<double>::infinity()) {
            continue;
        }
        options->push_back({node, option});
    }

    // recursively calls the function for all reachable nodes
    for (RRTNode* child : node->getReachable()) {
        fillOptions(options, child, end);
    }
}

void RRTTree::RRTStar(RRTNode* sample, double rewire_radius) {
    RRTStarRecursive(current_head, sample, rewire_radius);
}

RRTNode* RRTTree::getNearestNode(const RRTPoint& point) {
    RRTNode* nearest = nullptr;
    double min_distance = std::numeric_limits<double>::infinity();

    for (auto& [key, value] : node_map) {
        double distance = point.distanceTo(key);
        if (distance < min_distance) {
            min_distance = distance;
            nearest = value;
        }
    }

    return nearest;
}

void RRTTree::RRTStarRecursive(RRTNode* current_node, RRTNode* sample, double rewire_radius) {
    // base case
    if (current_node == nullptr) {
        return;
    }

    // recurse
    for (RRTNode* child : current_node->getReachable()) {
        RRTStarRecursive(child, sample, rewire_radius);
    }

    // for all nodes past the current node, attempt to rewire them
    for (RRTNode* child : current_node->getReachable()) {
        // get the distance between the current node and the nearest node
        if (child->getPoint().distanceTo(sample->getPoint()) > rewire_radius) {
            continue;
        }

        if (child->getPoint() == sample->getPoint()) {
            continue;
        }

        // get the dubins options (sorted)
        std::vector<RRTOption> options =
            dubins.allOptions(sample->getPoint(), child->getPoint(), true);

        // for each option
        for (const RRTOption& option : options) {
            // get the new cost
            if (std::isnan(option.length) ||
                option.length == std::numeric_limits<double>::infinity()) {
                break;
            }

            // if the node is uncompetitive, continue
            double new_cost = sample->getCost() + option.length;

            double cost = child->getCost();

            // if the new cost is less than the current cost
            if (new_cost < cost) {
                // check if new path is valid
                std::vector<XYZCoord> path = dubins.generatePoints(
                    sample->getPoint(), child->getPoint(), option.dubins_path, option.has_straight);

                if (!airspace.isPathInBounds(path)) {
                    continue;
                }

                // rewire the edge
                rewireEdge(child, current_node, sample, path, option.length);
            }
        }
    }
}
