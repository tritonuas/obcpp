#include "pathing/tree.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/logging.hpp"
#include "utilities/rng.hpp"
#include "utilities/obc_config.hpp"

// When we are born, we cry that we are come to this great stage of fools

RRTNode::RRTNode(const RRTPoint& point, double cost, double path_length,
                 const std::vector<XYZCoord> path)
    : point{point}, cost{cost}, path_length(path_length), path(path) {}

RRTNode::RRTNode(const RRTPoint& point, double cost, double path_length,
                 const std::vector<XYZCoord> path, std::vector<RRTNodePtr> reachable)
    : point{point}, cost{cost}, path_length(path_length), path(path), reachable{reachable} {}

RRTNode::~RRTNode() {
    reachable.clear();
}

// bool RRTNode::operator==(const RRTNode& other_node) const {
//     return this->point == other_node.point && this->cost == other_node.cost;
// }

RRTPoint& RRTNode::getPoint() { return this->point; }

void RRTNode::setReachable(const std::vector<RRTNodePtr>& reachable) {
    this->reachable = reachable;
}
void RRTNode::addReachable(RRTNodePtr new_node) {
    this->reachable.push_back(new_node);
}

void RRTNode::removeReachable(RRTNodePtr old_node) {
    for (int i = 0; i < reachable.size(); i++) {
        if (reachable.at(i) == old_node) {
            reachable.erase(reachable.begin() + i);
            return;
        }
    }
}

const std::vector<RRTNodePtr>& RRTNode::getReachable() { return (this->reachable); }

double RRTNode::getCost() const { return this->cost; }

void RRTNode::setCost(double new_cost) { this->cost = new_cost; }

const std::vector<XYZCoord>& RRTNode::getPath() const { return this->path; }

void RRTNode::setPath(const std::vector<XYZCoord>& path) { this->path = path; }

double RRTNode::getPathLength() const { return this->path_length; }

void RRTNode::setPathLength(double new_path_length) { this->path_length = new_path_length; }



/** RRTTree */

RRTTree::RRTTree(RRTPoint root_point, Environment airspace, Dubins dubins)
    : airspace(airspace), dubins(dubins), tree_size(1) {
    RRTNodePtr new_node = std::make_shared<RRTNode>(root_point, 0, 0, {});
    root = new_node;
    current_head = new_node;
}

RRTTree::~RRTTree() { root.reset(); }

// MAYBE: glorified function wrapper that uses one of its params, need to wipe.
bool RRTTree::validatePath(const std::vector<XYZCoord>& path, const RRTOption& option) const {
    return airspace.isPathInBounds(path);
}

RRTNodePtr RRTTree::generateNode(RRTNodePtr anchor_node, const RRTPoint& new_point,
                               const RRTOption& option) const {
    const std::vector<XYZCoord>& path = dubins.generatePoints(
        anchor_node->getPoint(), new_point, option.dubins_path, option.has_straight);

    if (!validatePath(path, option)) {
        return nullptr;
    }

    // needs to add the node to the tree
    RRTNodePtr new_node =
        std::make_shared<RRTNode>(new_point, anchor_node->getCost() + option.length, option.length, path);

    return new_node;
}

bool RRTTree::addNode(RRTNodePtr anchor_node, RRTNodePtr new_node) {
    if (new_node == nullptr || anchor_node == nullptr) {
        return false;
    }

    anchor_node->addReachable(new_node);
    tree_size++;
    return true;
}

// TODO - convert from old to new
RRTNodePtr RRTTree::addSample(RRTNodePtr anchor_node, const RRTPoint& new_point,
                            const RRTOption& option) {
    RRTNodePtr new_node = generateNode(anchor_node, new_point, option);

    if (addNode(anchor_node, new_node)) {
        return new_node;
    }

    return nullptr;
}

void RRTTree::fillOptionsNodes(std::vector<std::pair<RRTNodePtr, RRTOption>>& options,
                               const std::vector<RRTNodePtr>& nodes, const RRTPoint& sample) const {
    for (RRTNodePtr node : nodes) {
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

RRTNodePtr RRTTree::getRoot() const { return this->root; }

const XYZCoord& RRTTree::getGoal() const { return airspace.getGoal(); }

const XYZCoord& RRTTree::getGoal(int index) const { return airspace.getGoal(index); }

const Environment& RRTTree::getAirspace() const { return this->airspace; }

RRTPoint RRTTree::getRandomPoint(double search_radius) const {
    // gets random point if the goal is not being used
    const XYZCoord& sample = airspace.getRandomPoint();

    // MAYBE: go through this code, if anything of value, take it, otherwise throw away

    // // picks the nearest node to the sample, and then returns a point `search_radius` distance
    // away
    // // from tat point in the direction of the sample
    // std::pair<RRTNodePtr, double> nearest_node = getNearestNode(sample);

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
std::vector<std::pair<RRTNodePtr, RRTOption>> RRTTree::pathingOptions(
    const RRTPoint& end, PointFetchMethod::Enum point_fetch_method, int quantity_options) const {
    // fills the options list with valid values
    std::vector<std::pair<RRTNodePtr, RRTOption>> options;
    fillOptions(options, current_head, end);

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

void RRTTree::fillOptions(std::vector<std::pair<RRTNodePtr, RRTOption>>& options, RRTNodePtr node,
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
    for (RRTNodePtr child : node->getReachable()) {
        fillOptions(options, child, end);
    }
}

void RRTTree::setCurrentHead(RRTNodePtr goal) {
    if (goal == nullptr) {
        LOG_F(ERROR, "FAILURE: Goal is not in the tree");
        return;
    }

    // update local parametters
    tree_size = 1;
    current_head = goal;
}

// std::vector<XYZCoord> RRTTree::getPathToGoal() const {
//     RRTNodePtr current_node = current_head;
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

std::vector<XYZCoord> RRTTree::getPathSegment(RRTNodePtr node) const {
    RRTNodePtr current_node = node;
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

// I'm keeping this code for the time being, because it's prolly useful for implementing RRT
// // std::pair<RRTNodePtr, double> RRTTree::getNearestNode(const XYZCoord& point) const {
// //     RRTNodePtr nearest = nullptr;
// //     double min_distance = std::numeric_limits<double>::infinity();

// //     for (auto& node : node_set) {
// //         double distance = point.distanceToSquared(node->getPoint().coord);
// //         if (distance < min_distance) {
// //             min_distance = distance;
// //             nearest = node;
// //         }
// //     }

// //     return {nearest, min_distance};
// // }