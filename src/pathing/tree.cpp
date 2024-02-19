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

RRTNode::RRTNode(const RRTPoint &point, double cost) : point{point}, cost{cost} {}

RRTNode::RRTNode(const RRTPoint &point, double cost, RRTNodeList &reachable)
    : point{point}, cost{cost}, reachable{reachable} {}

bool RRTNode::operator==(const RRTNode& other_node) const {
    return this->point == other_node.point && this->cost == other_node.cost;
}

RRTPoint RRTNode::getPoint() { return this->point; }

void RRTNode::setReachable(const RRTNodeList &reachable) {
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

RRTEdge::RRTEdge(RRTNode* from, RRTNode* to, const std::vector<XYZCoord> &path, double cost)
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
    node_map.insert(std::make_pair(root_point, new_node));
}

// TODO - seems a bit sketchy
RRTTree::~RRTTree() {
    for (std::pair<RRTPoint, RRTNode*> node : node_map) {
        delete node.second;
    }
}

// TODO - convert from old to new
RRTNode* RRTTree::addNode(RRTNode* anchor_node, const RRTPoint& new_point, const RRTOption& option,
                          int goal_index) {
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

    // if goal is found, then it will retrieve a naive path
    if (airspace.isPointInGoal(new_point.coord, goal_index)) {
        airspace.setGoalfound(goal_index + 1);
        if (airspace.isPathComplete()) {
            retreivePathByNode(new_node, new_node->getParent());
        }
    }

    return new_node;
}

void RRTTree::rewireEdge(RRTNode* current_node, RRTNode* previous_parent, RRTNode* new_parent,
                         const std::vector<XYZCoord> &path, double cost) {
    // if the new_node doesn't have a parent, then this code will crash
    std::pair<RRTNode*, RRTNode*> previous_pair(previous_parent, current_node);
    std::pair<RRTNode*, RRTNode*> new_pair(new_parent, current_node);

    RRTEdge new_edge = RRTEdge(new_parent, current_node, path, cost);
    // replace prev node in "from" node's neighbor list with new node

    // based on testing, if current_point and new_connection are already connected
    // there will be duplicate edges in the edge_map

    // ORDER MATTERS, REMOVE THEN ADD TO PRESERVE THE CURR_NODE HAS A PARENT
    previous_parent->removeReachable(current_node);
    new_parent->addReachable(current_node);
    // current_node->setParent(new_parent);

    // remove old edge from edge_map, add the new edge
    edge_map.erase(previous_pair);
    edge_map.emplace(new_pair, new_edge);
    current_node->setCost(new_parent->getCost() + cost);

    reassignCosts(current_node);
}

// ALSO BROKEN
RRTNode* RRTTree::getNode(const RRTPoint &point) {
    if (node_map.count(point) > 0) {
        return node_map.at(point);
    } else {
        return nullptr;
    }
}

// BROKEN
RRTEdge* RRTTree::getEdge(const RRTPoint &from, const RRTPoint &to) {
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

RRTPoint RRTTree::getGoal() const { return airspace.getGoal(); }

RRTPoint RRTTree::getGoal(int index) const { return airspace.getGoal(index); }

Environment RRTTree::getAirspace() const { return this->airspace; }

RRTPoint RRTTree::getRandomPoint(double search_radius, bool use_goal) {
    // gets random point if the goal is not being used
    const RRTPoint& sample = use_goal ? getGoal() : airspace.getRandomPoint();

    // picks the nearest node to the sample, and then returns a point `search_radius` distance away
    // from tat point in the direction of the sample
    RRTNode* nearest_node = getNearestNode(sample);

    // shouldn't happen, it is here for memory safety
    if (nearest_node == nullptr) {
        return sample;
    }

    const RRTPoint& nearest_point = nearest_node->getPoint();

    const XYZCoord displacement_vector = sample.coord - nearest_point.coord;

    // TODO - use some heuristic to make this angle as optimal as possible
    const double angle = std::atan2(displacement_vector.y, displacement_vector.x);

    // distance between the vectors, if it is less than the search radius, then it will return the
    // sample point, otherwise, it will return a point `search_radius` away from the nearest point
    const double distance = displacement_vector.norm();

    if (distance < search_radius) {
        return sample;
    }

    RRTPoint new_point(nearest_point.coord + (search_radius / distance) * displacement_vector,
                       angle);

    return new_point;
}

std::vector<std::pair<RRTNode*, RRTOption>> RRTTree::pathingOptions(const RRTPoint& end,
                                                                    int quantity_options) {
    // fills the options list with valid values
    std::vector<std::pair<RRTNode*, RRTOption>> options;
    fillOptions(&options, current_head, end);

    // sorts the list
    std::sort(options.begin(), options.end(),
              [](auto a, auto b) { return compareRRTOptionLength(a.second, b.second); });

    // if there are less options than needed amount, then just reurn the xisting list, else,
    // return a truncated list.
    if (options.size() < quantity_options) {
        return options;
    }
    // erases everythin after the last wanted element
    options.erase(options.begin() + quantity_options, options.end());

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
        if (option.length != std::numeric_limits<double>::infinity()) {
            options->emplace_back(std::pair<RRTNode*, RRTOption>{node, option});
        }
    }

    // recursively calls the function for all reachable nodes
    for (RRTNode* child : node->getReachable()) {
        fillOptions(options, child, end);
    }
}

std::vector<XYZCoord> RRTTree::getPathToGoal(bool without_cache) {
    if (without_cache) {
        std::vector<std::pair<std::vector<RRTNode*>, double>> path_options{};

        // run DFS
        for (RRTNode* child : root->getReachable()) {
            std::vector<RRTNode*> current_path;
            fillPathOptionsRecursive(path_options, child, root, 0, current_path);
        }

        // if there are no paths, then return an empty path
        if (path_options.size() == 0) {
            return {};
        }

        // sort path by shortest path length
        std::sort(path_options.begin(), path_options.end(),
                  [](auto a, auto b) { return a.second < b.second; });

        // gets the first path in path_options
        std::vector<RRTNode*> path_nodes = path_options[0].first;

        // generate the path
        std::vector<XYZCoord> path;

        for (int i = 0; i < path_nodes.size() - 1; i++) {
            RRTNode* node1 = path_nodes[i];
            RRTNode* node2 = path_nodes[i + 1];

            RRTEdge edge = edge_map.at(std::make_pair(node1, node2));

            // fails to connect nodes should probably throw error because this should never happen
            path.insert(path.end(), edge.getPath().begin(), edge.getPath().end());
        }
        return path;
    }

    if (!airspace.isPathComplete() || path_to_goal.size() == 0) {
        return {};  // should probably throw an error
    }

    return path_to_goal;
}

void RRTTree::RRTStar(RRTNode* sample, double rewire_radius) {
    RRTStarRecursive(current_head, sample, rewire_radius);
}

void RRTTree::retreivePathByNode(RRTNode* node, RRTNode* parent) {
    if (parent == nullptr || node == nullptr) {
        return;
    }

    retreivePathByNode(parent, parent->getParent());

    // STEG FAULS GET EDGE, SOMETHING IS WRONG IN THE EDGE MAP OR GET NODE
    // retreive's path from the edge_map
    std::vector<XYZCoord> edge_path = edge_map.at(std::make_pair(parent, node)).getPath();

    // since this is at the end of the recuraive chain, it adds to the end of the path_to_goal
    path_to_goal.insert(path_to_goal.end(), edge_path.begin(), edge_path.end());
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
    for (RRTNode* node : current_node->getReachable()) {
        // get the distance between the current node and the nearest node
        if (node->getPoint().distanceTo(sample->getPoint()) > rewire_radius) {
            continue;
        }

        if (node->getPoint() == sample->getPoint()) {
            continue;
        }

        // get the dubins options
        std::vector<RRTOption> options =
            dubins.allOptions(sample->getPoint(), node->getPoint(), true);

        // for each option
        for (const RRTOption& option : options) {
            // get the new cost
            if (std::isnan(option.length) ||
                option.length == std::numeric_limits<double>::infinity()) {
                break;
            }

            // check if new path is valid
            std::vector<XYZCoord> path = dubins.generatePoints(
                sample->getPoint(), node->getPoint(), option.dubins_path, option.has_straight);

            if (!airspace.isPathInBounds(path)) {
                continue;
            }

            // if the node is uncompetitive, continue
            double new_cost = sample->getCost() + option.length;

            double cost = node->getCost();

            // if the new cost is less than the current cost
            if (new_cost < cost) {
                // rewire the edge
                rewireEdge(node, current_node, sample, path, option.length);
            }
        }
    }
}
