#include "pathing/tree.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

RRTNode::RRTNode(const RRTPoint& point, double cost, double path_length,
                 const std::vector<XYZCoord> path)
    : point{point}, cost{cost}, path_length(path_length), path(path) {}

RRTNode::RRTNode(const RRTPoint& point, double cost, double path_length,
                 const std::vector<XYZCoord> path, RRTNodeList reachable)
    : point{point}, cost{cost}, path_length(path_length), path(path), reachable{reachable} {}

// bool RRTNode::operator==(const RRTNode& other_node) const {
//     return this->point == other_node.point && this->cost == other_node.cost;
// }

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

const std::vector<XYZCoord>& RRTNode::getPath() const { return this->path; }

void RRTNode::setPath(const std::vector<XYZCoord>& path) { this->path = path; }

double RRTNode::getPathLength() const { return this->path_length; }

void RRTNode::setPathLength(double new_path_length) { this->path_length = new_path_length; }
/*




*/
/** RRTTree */
/*




*/

RRTTree::RRTTree(RRTPoint root_point, Environment airspace, Dubins dubins,
                 PATH_OPTIONS point_fetch_choice)
    : airspace(airspace), dubins(dubins), point_fetch_choice(point_fetch_choice) {
    RRTNode* new_node = new RRTNode(root_point, 0, 0, {});
    root = new_node;
    current_head = new_node;
}

// TODO - seems a bit sketchy
RRTTree::~RRTTree() { deleteTree(root); }

void RRTTree::deleteTree(RRTNode* node) {
    if (node == nullptr) {
        return;
    }

    for (RRTNode* child : node->getReachable()) {
        deleteTree(child);
    }

    delete node;
}

bool RRTTree::validatePath(const std::vector<XYZCoord>& path, const RRTOption& option) const {
    return airspace.isPathInBoundsAdv(path, option);
}

RRTNode* RRTTree::generateNode(RRTNode* anchor_node, const RRTPoint& new_point,
                               const RRTOption& option) const {
    const std::vector<XYZCoord>& path = dubins.generatePoints(
        anchor_node->getPoint(), new_point, option.dubins_path, option.has_straight);

    if (!validatePath(path, option)) {
        // if (!airspace.isPathInBounds(path)) {
        return nullptr;
    }

    return new RRTNode(new_point, anchor_node->getCost() + option.length, option.length, path);
}

// TODO - convert from old to new
RRTNode* RRTTree::addSample(RRTNode* anchor_node, const RRTPoint& new_point,
                            const RRTOption& option) {
    // checking if path is valid
    const std::vector<XYZCoord>& path = dubins.generatePoints(
        anchor_node->getPoint(), new_point, option.dubins_path, option.has_straight);

    if (!validatePath(path, option)) {
        // if (!airspace.isPathInBounds(path)) {
        return nullptr;
    }

    // if a valid path was found, it will add the node to the tree
    RRTNode* new_node =
        new RRTNode(new_point, anchor_node->getCost() + option.length, option.length, path);

    if (std::isnan(new_node->getCost())) {
        std::cout << "Cost is nan" << std::endl;
    }

    // changing state of its parent
    anchor_node->addReachable(new_node);
    return new_node;
}

void RRTTree::rewireEdge(RRTNode* current_node, RRTNode* previous_parent, RRTNode* new_parent,
                         const std::vector<XYZCoord>& path, double path_cost) {
    // remove old edge from edge_map, add the new edge
    // ORDER MATTERS, REMOVE THEN ADD TO PRESERVE THE CURR_NODE HAS A PARENT
    previous_parent->removeReachable(current_node);
    new_parent->addReachable(current_node);

    current_node->setCost(new_parent->getCost() + path_cost);

    reassignCosts(current_node);
}

// BROKEN
// RRTEdge* RRTTree::getEdge(const RRTPoint& from, const RRTPoint& to) {
//     RRTNode* node1 = getNode(from);
//     RRTNode* node2 = getNode(to);
//     if (node1 == nullptr || node2 == nullptr) {
//         return nullptr;
//     }

//     std::pair<RRTNode*, RRTNode*> edgePair(node1, node2);
//     if (edge_map.count(edgePair) > 0) {
//         return &(edge_map.at(edgePair));
//     } else {
//         return nullptr;
//     }
// }

RRTNode* RRTTree::getRoot() const { return this->root; }

XYZCoord RRTTree::getGoal() const { return airspace.getGoal(); }

XYZCoord RRTTree::getGoal(int index) const { return airspace.getGoal(index); }

Environment RRTTree::getAirspace() const { return this->airspace; }

RRTPoint RRTTree::getRandomPoint(double search_radius) const {
    // gets random point if the goal is not being used
    XYZCoord sample = airspace.getRandomPoint();

    // // picks the nearest node to the sample, and then returns a point `search_radius` distance
    // away
    // // from tat point in the direction of the sample
    // std::pair<RRTNode*, double> nearest_node = getNearestNode(sample);

    // int count = 0;

    // while (nearest_node.second < search_radius && count < TRIES_FOR_RANDOM_POINT) {
    //     sample = airspace.getRandomPoint();
    //     nearest_node = getNearestNode(sample);
    //     count++;
    // }
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
    return RRTPoint(sample, random(0, TWO_PI));
}

std::vector<std::pair<RRTNode*, RRTOption>> RRTTree::pathingOptions(const RRTPoint& end,
                                                                    int quantity_options) const {
    // fills the options list with valid values
    std::vector<std::pair<RRTNode*, RRTOption>> options;

    switch (point_fetch_choice) {
        case PATH_OPTIONS::RANDOM: {
            const std::vector<RRTNode*>& nodes = getKRandomNodes(end, 100);
            fillOptionsNodes(options, nodes, end);
        } break;
        case PATH_OPTIONS::NEAREST: {
            const std::vector<RRTNode*>& nodes = getKClosestNodes(end, 100);
            fillOptionsNodes(options, nodes, end);
        } break;
            // actually uses switch statements functionality, it should default to doing
            // the most accurate but slow method
        case PATH_OPTIONS::NONE:
            fillOptions(options, current_head, end);
            break;
        default:
            fillOptions(options, current_head, end);
            break;
    }

    const std::vector<RRTNode*>& nodes = getKClosestNodes(end, 100);
    fillOptionsNodes(options, nodes, end);

    // sorts the list
    std::sort(options.begin(), options.end(), [](auto a, auto b) {
        auto& [a_node, a_option] = a;
        auto& [b_node, b_option] = b;
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

// only get the first 10 nodes
void RRTTree::fillOptions(std::vector<std::pair<RRTNode*, RRTOption>>& options, RRTNode* node,
                          const RRTPoint& end) const {
    /*
       TODO - try to limit the scope of the search to prevent too many calls to dubins
    */
    if (node == nullptr) {
        return;
    }

    // gets all dubins curves from the current node to the end point
    const std::vector<RRTOption>& local_options = dubins.allOptions(node->getPoint(), end);

    // filters out the options that are not valid
    for (const RRTOption& option : local_options) {
        if (std::isnan(option.length) || option.length == std::numeric_limits<double>::infinity()) {
            continue;
        }

        options.push_back({node, option});
    }

    // RRTOption best_option = dubins.bestOption(node->getPoint(), end);
    // if (!std::isnan(best_option.length) &&
    //     best_option.length != std::numeric_limits<double>::infinity()) {
    //     options.push_back({node, best_option});
    // }

    // recursively calls the function for all reachable nodes
    for (RRTNode* child : node->getReachable()) {
        fillOptions(options, child, end);
    }
}

void RRTTree::RRTStar(RRTNode* sample, double rewire_radius) {
    RRTStarRecursive(current_head, sample, rewire_radius);
}

void RRTTree::setCurrentHead(RRTNode* goal) {
    if (goal == nullptr) {
        std::cout << "FAILURE: Goal is not in the tree\n" << std::endl;
        return;
    }

    // prune the tree
    RRTNode* current_node = goal;
    RRTNode* parent_node = goal->getParent();
    while (parent_node != nullptr) {
        for (RRTNode* child : parent_node->getReachable()) {
            if (child != current_node) {
                parent_node->removeReachable(child);
            }
        }

        current_node = parent_node;
        parent_node = parent_node->getParent();
    }

    current_head = goal;
}

std::vector<XYZCoord> RRTTree::getPathToGoal() const {
    RRTNode* current_node = current_head;
    std::vector<XYZCoord> path = {};

    std::cout << "Getting path to goal" << std::endl;

    while (current_node != nullptr && current_node->getParent() != nullptr) {
        std::vector<XYZCoord> edge_path = current_node->getPath();

        // TODO - misses first node
        path.insert(path.begin(), edge_path.begin() + 1, edge_path.end());
        current_node = current_node->getParent();
    }

    return path;
}

/* RRTTree Private */

// std::pair<RRTNode*, double> RRTTree::getNearestNode(const XYZCoord& point) const {
//     RRTNode* nearest = nullptr;
//     double min_distance = std::numeric_limits<double>::infinity();

//     for (auto& node : node_set) {
//         double distance = point.distanceToSquared(node->getPoint().coord);
//         if (distance < min_distance) {
//             min_distance = distance;
//             nearest = node;
//         }
//     }

//     return {nearest, min_distance};
// }

void RRTTree::RRTStarRecursive(RRTNode* current_node, RRTNode* sample, double rewire_radius) {
    // base case
    if (current_node == nullptr) {
        return;
    }

    // for all nodes past the current node, attempt to rewire them
    for (RRTNode* child : current_node->getReachable()) {
        // get the distance between the current node and the nearest node
        if (child->getPoint().distanceToSquared(sample->getPoint()) >
            rewire_radius * rewire_radius) {
            continue;
        }

        // the child shouldn't have any children
        if (child == sample) {
            return;
        }

        // get the dubins options (sorted)
        const std::vector<RRTOption>& options =
            dubins.allOptions(sample->getPoint(), child->getPoint(), true);
        // RRTOption option = dubins.bestOption(sample->getPoint(), child->getPoint());

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

            if (new_cost >= cost) {
                break;
            }

            // if the new cost is less than the current cost
            // check if new path is valid
            const std::vector<XYZCoord>& path = dubins.generatePoints(
                sample->getPoint(), child->getPoint(), option.dubins_path, option.has_straight);

            if (!airspace.isPathInBoundsAdv(path, option)) {
                // if (!airspace.isPathInBounds(path)) {
                continue;
            }

            // rewire the edge
            rewireEdge(child, current_node, sample, path, option.length);
        }
    }

    // recurse
    for (RRTNode* child : current_node->getReachable()) {
        RRTStarRecursive(child, sample, rewire_radius);
    }
}

void RRTTree::reassignCosts(RRTNode* changed_node) {
    if (changed_node == nullptr) {
        return;
    }

    for (RRTNode* child : changed_node->getReachable()) {
        reassignCostsRecursive(changed_node, child, changed_node->getCost());
    }
}

void RRTTree::reassignCostsRecursive(RRTNode* parent, RRTNode* current_node, double path_cost) {
    if (current_node == nullptr) {
        return;
    }

    current_node->setCost(path_cost + current_node->getPathLength());
    for (RRTNode* neighbor : current_node->getReachable()) {
        reassignCostsRecursive(current_node, neighbor, current_node->getCost());
    }
}
