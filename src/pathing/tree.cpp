#include "pathing/tree.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/logging.hpp"
#include "utilities/rng.hpp"

RRTNode::RRTNode(const RRTPoint& point, double cost, double path_length,
                 const std::vector<XYZCoord> path)
    : point{point}, cost{cost}, path_length(path_length), path(path) {}

RRTNode::RRTNode(const RRTPoint& point, double cost, double path_length,
                 const std::vector<XYZCoord> path, RRTNodeList reachable)
    : point{point}, cost{cost}, path_length(path_length), path(path), reachable{reachable} {}

RRTNode::~RRTNode() {
    for (RRTNode* node : reachable) {
        delete node;
    }
}

// bool RRTNode::operator==(const RRTNode& other_node) const {
//     return this->point == other_node.point && this->cost == other_node.cost;
// }

RRTPoint& RRTNode::getPoint() { return this->point; }

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

RRTTree::RRTTree(RRTPoint root_point, Environment airspace, Dubins dubins)
    : airspace(airspace), dubins(dubins), tree_size(1) {
    RRTNode* new_node = new RRTNode(root_point, 0, 0, {});
    root = new_node;
    current_head = new_node;
}

// TODO - seems a bit sketchy
RRTTree::~RRTTree() { delete root; }

bool RRTTree::validatePath(const std::vector<XYZCoord>& path, const RRTOption& option) const {
    // return airspace.isPathInBounds(path);
    return airspace.isPathInBoundsAdv(path, option);
}

RRTNode* RRTTree::generateNode(RRTNode* anchor_node, const RRTPoint& new_point,
                               const RRTOption& option) const {
    const std::vector<XYZCoord>& path = dubins.generatePoints(
        anchor_node->getPoint(), new_point, option.dubins_path, option.has_straight);

    if (!validatePath(path, option)) {
        return nullptr;
    }

    // needs to add the node to the tree
    RRTNode* new_node =
        new RRTNode(new_point, anchor_node->getCost() + option.length, option.length, path);
    new_node->setParent(anchor_node);

    return new_node;
}

bool RRTTree::addNode(RRTNode* anchor_node, RRTNode* new_node) {
    if (new_node == nullptr || anchor_node == nullptr) {
        return false;
    }

    anchor_node->addReachable(new_node);
    tree_size++;
    return true;
}

// TODO - convert from old to new
RRTNode* RRTTree::addSample(RRTNode* anchor_node, const RRTPoint& new_point,
                            const RRTOption& option) {
    RRTNode* new_node = generateNode(anchor_node, new_point, option);

    if (addNode(anchor_node, new_node)) {
        return new_node;
    }

    return nullptr;
}

void RRTTree::rewireEdge(RRTNode* current_node, RRTNode* previous_parent, RRTNode* new_parent,
                         const std::vector<XYZCoord>& path, double path_cost) {
    // ORDER MATTERS, REMOVE THEN ADD TO PRESERVE THE CURR_NODE HAS A PARENT
    previous_parent->removeReachable(current_node);
    new_parent->addReachable(current_node);

    // bubbles down the tree to reassign the costs
    current_node->setPath(path);
    current_node->setCost(new_parent->getCost() + path_cost);
    current_node->setPathLength(path_cost);
    reassignCosts(current_node);
}

std::vector<RRTNode*> RRTTree::getKRandomNodes(int k) const {
    std::vector<RRTNode*> nodes;
    // proabability that any given node should be added
    double chance = 1.0 * k / tree_size;
    getKRandomNodesRecursive(nodes, current_head, chance);

    return nodes;
}

void RRTTree::getKRandomNodesRecursive(std::vector<RRTNode*>& nodes, RRTNode* current_node,
                                       double chance) const {
    if (current_node == nullptr) {
        return;
    }

    // if the chance is less than the random number, then add the node to the list
    // TODO maybe make some check that prevents the random calls if the tree is small enough
    if (random(0, 1) < chance) {
        nodes.emplace_back(current_node);
    }

    for (RRTNode* node : current_node->getReachable()) {
        getKRandomNodesRecursive(nodes, node, chance);
    }
}

std::vector<RRTNode*> RRTTree::getKClosestNodes(const RRTPoint& sample, int k) const {
    std::vector<RRTNode*> closest_nodes;

    // helper vector that associates nodes with distances
    // TODO - do some benchmarks with max-heaps to see which one is more efficient
    std::vector<std::pair<double, RRTNode*>> nodes_by_distance;
    getKClosestNodesRecursive(nodes_by_distance, sample, current_head);

    // sorts the nodes by distance
    std::sort(nodes_by_distance.begin(), nodes_by_distance.end(),
              [](auto& left, auto& right) { return left.first < right.first; });

    // gets either the k closest nodes, or the entire list
    int size = nodes_by_distance.size();
    int stop_condition = std::min(k, size);
    for (int i = 0; i < stop_condition; i++) {
        closest_nodes.emplace_back(nodes_by_distance[i].second);
    }

    return closest_nodes;
}

void RRTTree::getKClosestNodesRecursive(std::vector<std::pair<double, RRTNode*>>& nodes_by_distance,
                                        const RRTPoint& sample, RRTNode* current_node) const {
    if (current_node == nullptr) {
        return;
    }

    // ONLY considers the distance, and not the path length required to get to the node
    double distance = sample.coord.distanceToSquared(current_node->getPoint().coord);
    nodes_by_distance.push_back({distance, current_node});

    for (RRTNode* node : current_node->getReachable()) {
        getKClosestNodesRecursive(nodes_by_distance, sample, node);
    }
}

void RRTTree::fillOptionsNodes(std::vector<std::pair<RRTNode*, RRTOption>>& options,
                               const std::vector<RRTNode*>& nodes, const RRTPoint& sample) const {
    for (RRTNode* node : nodes) {
        const std::vector<RRTOption>& local_options = dubins.allOptions(node->getPoint(), sample);

        for (const RRTOption& option : local_options) {
            if (std::isnan(option.length) ||
                option.length == std::numeric_limits<double>::infinity()) {
                continue;
            }

            options.push_back({node, option});
        }
    }
}

RRTNode* RRTTree::getRoot() const { return this->root; }

const XYZCoord& RRTTree::getGoal() const { return airspace.getGoal(); }

const XYZCoord& RRTTree::getGoal(int index) const { return airspace.getGoal(index); }

const Environment& RRTTree::getAirspace() const { return this->airspace; }

RRTPoint RRTTree::getRandomPoint(double search_radius) const {
    // gets random point if the goal is not being used
    const XYZCoord& sample = airspace.getRandomPoint();

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

/*
    TODO - investigate whether a max heap is better or worse
*/
std::vector<std::pair<RRTNode*, RRTOption>> RRTTree::pathingOptions(
    const RRTPoint& end, POINT_FETCH_METHODS point_fetch_method, int quantity_options) const {
    // fills the options list with valid values
    std::vector<std::pair<RRTNode*, RRTOption>> options;

    switch (point_fetch_method) {
        case POINT_FETCH_METHODS::RANDOM: {
            const std::vector<RRTNode*>& nodes = getKRandomNodes(K_RANDOM_NODES);
            fillOptionsNodes(options, nodes, end);
        } break;
        case POINT_FETCH_METHODS::NEAREST: {
            const std::vector<RRTNode*>& nodes = getKClosestNodes(end, K_CLOESEST_NODES);
            fillOptionsNodes(options, nodes, end);
        } break;
        case POINT_FETCH_METHODS::NONE:
            fillOptions(options, current_head, end);
            break;
        default:
            fillOptions(options, current_head, end);
            break;
    }

    // sorts the list
    std::sort(options.begin(), options.end(), [](auto& a, auto& b) {
        auto& [a_node, a_option] = a;
        auto& [b_node, b_option] = b;
        return a_option.length + a_node->getCost() < b_option.length + b_node->getCost();
    });

    // the options are already sorted, why return a truncated list?
    // 2024-03-11 : because pathing to goal req a certain amount, you can change this later if you
    // want, idk c++ memory management well enough to know if tht is a good idea (for speed)

    // if there are less options than needed amount, then just reurn the xisting list, else,
    // return a truncated list.
    if (options.size() < quantity_options) {
        return options;
    }

    // erases everything after the last wanted element
    options.erase(options.begin() + quantity_options, options.end());

    return options;
}

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

    // recursively calls the function for all reachable nodes
    for (RRTNode* child : node->getReachable()) {
        fillOptions(options, child, end);
    }
}

void RRTTree::RRTStar(RRTNode* sample, double rewire_radius) {
    // last element takes in the squared value of rewire_radius to prevent the need for sqrt()
    RRTStarRecursive(current_head, sample, rewire_radius * rewire_radius);
}

void RRTTree::setCurrentHead(RRTNode* goal) {
    if (goal == nullptr) {
        LOG_F(ERROR, "FAILURE: Goal is not in the tree");
        return;
    }

    // update local parametters
    tree_size = 1;
    current_head = goal;
}

// std::vector<XYZCoord> RRTTree::getPathToGoal() const {
//     RRTNode* current_node = current_head;
//     std::vector<XYZCoord> path = {};

//     while (current_node != nullptr && current_node->getParent() != nullptr) {
//         const std::vector<XYZCoord>& edge_path = current_node->getPath();

//         path.insert(path.begin(), edge_path.begin() + 1, edge_path.end());
//         current_node = current_node->getParent();
//     }

//     // loop above misses the first node, this adds it manually
//     path.insert(path.begin(), current_node->getPoint().coord);
//     return path;
// }

std::vector<XYZCoord> RRTTree::getPathSegment(RRTNode* node) const {
    RRTNode* current_node = node;
    std::vector<XYZCoord> path = {};

    while (current_node != current_head) {
        if (current_node == nullptr) {
            LOG_F(ERROR, "TREE: Segement does not find a path");
            return {};
        }

        const std::vector<XYZCoord>& edge_path = current_node->getPath();

        path.insert(path.begin(), edge_path.begin() + 1, edge_path.end());
        current_node = current_node->getParent();
    }

    return path;
}

RRTPoint& RRTTree::getStart() const { return root->getPoint(); }

/*-----------------*/
/* RRTTree Private */
/*-----------------*/

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

void RRTTree::RRTStarRecursive(RRTNode* current_node, RRTNode* sample,
                               double rewire_radius_squared) {
    // base case
    if (current_node == nullptr) {
        return;
    }

    // for all nodes past the current node, attempt to rewire them
    for (RRTNode* child : current_node->getReachable()) {
        // get the distance between the current node and the nearest node
        if (child->getPoint().distanceToSquared(sample->getPoint()) > rewire_radius_squared) {
            continue;
        }

        // the sample shouldn't have any children
        if (child == sample) {
            return;
        }

        // get the dubins options (sorted)
        const std::vector<RRTOption>& options =
            dubins.allOptions(sample->getPoint(), child->getPoint(), true);

        // for each option
        for (const RRTOption& option : options) {
            if (std::isnan(option.length) ||
                option.length == std::numeric_limits<double>::infinity()) {
                break;
            }

            // if the node is uncompetitive, move onto the next node
            double new_cost = sample->getCost() + option.length;
            double cost = child->getCost();

            if (new_cost >= cost) {
                break;
            }

            // if the new cost is less than the current cost
            // check if new path is valid
            const std::vector<XYZCoord>& path = dubins.generatePoints(
                sample->getPoint(), child->getPoint(), option.dubins_path, option.has_straight);

            if (!validatePath(path, option)) {
                continue;
            }

            // rewire the edge
            rewireEdge(child, current_node, sample, path, option.length);
        }
    }

    // recurse
    for (RRTNode* child : current_node->getReachable()) {
        RRTStarRecursive(child, sample, rewire_radius_squared);
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

    // reassigns the cost: cost to get to the parent + known path length between parent and child
    current_node->setCost(path_cost + current_node->getPathLength());
    for (RRTNode* neighbor : current_node->getReachable()) {
        reassignCostsRecursive(current_node, neighbor, current_node->getCost());
    }
}
