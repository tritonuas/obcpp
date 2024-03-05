#ifndef INCLUDE_PATHING_TREE_HPP_
#define INCLUDE_PATHING_TREE_HPP_

#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

class RRTNode;
typedef std::vector<RRTNode*> RRTNodeList;
typedef XYZCoord Vector;

// Define a custom comparison function
struct CustomCompare {
    CustomCompare() = default;
    bool operator()(const std::pair<double, RRTNode*>& a, const std::pair<double, RRTNode*>& b) {
        // Define your custom comparison logic here
        // For example, let's say we want a max heap
        return a.first < b.first;  // Change to a > b for max heap
    }
};

class RRTNode {
 public:
    RRTNode(const RRTPoint& point, double cost);
    RRTNode(const RRTPoint& point, double cost, RRTNodeList& reachable);

    /*
     *  Equality overload method for RRTNode object
     */
    bool operator==(const RRTNode& other_node) const;

    /*
     *  Get the RRTPoint associated with this RRTNode object
     */
    RRTPoint getPoint();

    /*
     *  Set the reachable (neighbors) list for this RRTNode object
     */
    void setReachable(const RRTNodeList& reachable);

    /*
     *  Add a new node to the end of this node's reachable list.
     *  Set the new node's parent to be this node.
     */
    void addReachable(RRTNode* new_node);

    /*
     *  Remove a specific node from this node's reachable list.
     *  Set the removed node's parent pointer to be null.
     */
    void removeReachable(RRTNode* old_node);

    /*
     *  Return a reference to this node's reachable list
     */
    const RRTNodeList& getReachable();

    /*
     *  Get the cost associated with this node
     */
    double getCost() const;

    /*
     *  Set the cost associated with this node
     */
    void setCost(double new_cost);

    /**
     * Get the parent of this node
     *
     * @return RRTNode* pointer to parent node
     */
    RRTNode* getParent() const;

    /**
     * Set the parent of this node
     *
     * @param new_parent pointer to new parent node
     */
    void setParent(RRTNode* new_parent);

 private:
    RRTPoint point;
    RRTNodeList reachable{};
    double cost;
    RRTNode* parent{};
};

class EdgeHashFunction {
 public:
    /*
     *  Hashes RRTNode* pair using the Cantor Pairing Function and RRTPoint hash
     * function. Used to add elements to unordered_map edgeMap in RRTTree.
     */
    std::size_t operator()(const std::pair<RRTNode*, RRTNode*>& node_pair) const;
};

class RRTEdge {
 public:
    RRTEdge(RRTNode* from, RRTNode* to, const std::vector<Vector>& path, double cost);

    /*
     *  Equality overload method for RRTEdge comparison
     */
    bool operator==(const RRTEdge& other_edge) const;

    /*
     *  Set the cost of this edge
     */
    void setCost(double new_cost);

    /*
     *  Get the cost associated with this edge
     */
    double getCost() const;

    /*
     *  Get the path vector associated with this edge
     */
    const std::vector<Vector>& getPath();

    /*
     *  Set the path vector associated with this edge
     */
    void setPath(std::vector<Vector> path);

 private:
    RRTNode* from;
    RRTNode* to;
    double cost;
    std::vector<Vector> path{};
};

class RRTTree {
 public:
    RRTTree(RRTPoint root_point, Environment airspace, Dubins dubins);
    ~RRTTree();

    /*
     *  Add a node to the RRTTree.
     *  If adding the first node to the tree, connectTo can be anything.
     */
    RRTNode* addNode(RRTNode* anchor_node, const RRTPoint& new_point, const RRTOption& option);

    /*

     */
    void rewireEdge(RRTNode* current_point, RRTNode* previous_connection, RRTNode* new_connection,
                    const std::vector<Vector>& path, double path_cost);

    /*
     *  Returns a pointer to the node in the tree corresponding to the RRTPoint.
     *  If the node doesn't exist in the tree, returns nullptr.
     */
    RRTNode* getNode(const RRTPoint& point) const;

    /*
     *  Returns a pointer to the edge in the tree corresponding to the point
     * pair. If the edge doesn't exist in the tree, returns nullptr.
     */
    // RRTEdge* getEdge(const RRTPoint& from, const RRTPoint& to);

    /**
     * Returns a pointer to the edge in the tree corresponding to the node pair.
     *
     * @param from  ==> the starting node
     * @param to    ==> the ending node
     * @return      ==> pointer to the edge
     */
    RRTEdge getEdge(RRTNode* from, RRTNode* to) const;

    /**
     * Returns a pointer to the root node
     *
     * @return RRTNode* pointer to root node
     */
    RRTNode* getRoot() const;

    /**
     * Get goal point
     *
     * @return RRTPoint goal point
     */
    XYZCoord getGoal() const;

    /**
     * Get goal point
     *
     * @return RRTPoint goal point
     */
    XYZCoord getGoal(int index) const;

    /**
     * returns the Environment object
     *
     * @return Environment object
     */
    Environment getAirspace() const;

    /**
     * Gets a random point in the environment
     *
     * @return RRTPoint random point in environment
     */
    RRTPoint getRandomPoint(double search_radius) const;

    /**
     * Returns a sorted list of the paths to get from a given node to the sampled
     * node
     *
     * @param end               ==> the sampled node that needs to be connected
     *                              to the tree
     * @param quantity_options  ==> the number of results to return back to the
     *                              function
     * @return                  ==> mininum sorted list of pairs of <node, path>
     */
    std::vector<std::pair<RRTNode*, RRTOption>> pathingOptions(
        const RRTPoint& end, int quantity_options = MAX_DUBINS_OPTIONS_TO_PARSE) const;

    /**
     * traverses the tree, and puts in all RRTOptions from dubins into a list
     * (DFS)
     *
     * @param options   ==> The list of options that is meant to be filled
     * @param node      ==> current node that will be traversed (DFS)
     * @param sample    ==> the end point that the options will be connected to
     */
    void fillOptions(std::vector<std::pair<RRTNode*, RRTOption>>& options, RRTNode* node,
                     const RRTPoint& sample) const;

    /** DOES RRT* for the program
     *
     * @param sample          ==> the point to used as the base
     * @param rewire_radius   ==> the radius to search for nodes to rewire
     */
    void RRTStar(RRTNode* sample, double rewire_radius);

    /**
     * Returns the currenthead of the tree
     *
     * @return RRTNode* pointer to the current head
     */
    RRTNode* getCurrentHead() const { return current_head; }

    /**
     * Changes the currentHead to the given goal
     *
     * @param goal ==> the goal to change the currentHead to
     */
    void setCurrentHead(RRTNode* goal);

    /**
     * Returns a path to the goal from the root
     *
     * The currentHead must be the goal for this to properly
     * generate a complete path
     * @return  ==> list of 2-vectors to the goal region
     */
    std::vector<XYZCoord> getPathToGoal() const;

    std::vector<RRTNode*> getKRandomNodes(const RRTPoint& sample, int k) const {
        std::vector<RRTNode*> nodes;   
        std::vector<RRTNode*> random_nodes;
        getKRandomNodesRecursive(nodes, current_head, k);

        int size = nodes.size();

        if (size <= k) {
            return nodes;
        }

        double chance = k / size;

        for (int i = 0; i < size; i++) {
            if (size - i > k - random_nodes.size()) {
                random_nodes.emplace_back(nodes[i]);
            } else {
                if (random(0, 1) < chance) {
                    random_nodes.emplace_back(nodes[i]);
                }
            }

            if (random_nodes.size() == k) {
                break;
            }
        }

        return random_nodes;
    }

    void getKRandomNodesRecursive(std::vector<RRTNode*>& nodes, RRTNode* current_node, int k) const {
        if (current_node == nullptr) {
            return;
        }

        nodes.emplace_back(current_node);

        for (RRTNode* node : current_node->getReachable()) {
            getKRandomNodesRecursive(nodes, node, k);
        }
    }

    std::vector<RRTNode*> getKClosestNodes(const RRTPoint& sample, int k) const {
        std::vector<RRTNode*> closest_nodes;
        std::vector<std::pair<double, RRTNode*>> nodes_by_distance;
        getKClosestNodesRecursive(nodes_by_distance, sample, current_head);
        // Get the k closest nodes from the priority queue
        std::sort(nodes_by_distance.begin(), nodes_by_distance.end(),
                  [](auto& left, auto& right) { return left.first < right.first; });

        int size = nodes_by_distance.size();
        int stop_condition = std::min(k, size);
        for (int i = 0; i < stop_condition; i++) {
            closest_nodes.emplace_back(nodes_by_distance[i].second);
        }

        return closest_nodes;
    }

    void getKClosestNodesRecursive(std::vector<std::pair<double, RRTNode*>>& nodes_by_distance,
                                   const RRTPoint& sample, RRTNode* current_node) const {
        if (current_node == nullptr) {
            return;
        }

        double distance = sample.coord.distanceToSquared(current_node->getPoint().coord);
        nodes_by_distance.push_back({distance, current_node});

        for (RRTNode* node : current_node->getReachable()) {
            getKClosestNodesRecursive(nodes_by_distance, sample, node);
        }
    }

    void fillOptionsNodes(std::vector<std::pair<RRTNode*, RRTOption>>& options,
                          const std::vector<RRTNode*>& nodes, const RRTPoint& sample) const {
        for (RRTNode* node : nodes) {
            const std::vector<RRTOption>& local_options =
                dubins.allOptions(node->getPoint(), sample);

            for (const RRTOption& option : local_options) {
                if (std::isnan(option.length) ||
                    option.length == std::numeric_limits<double>::infinity()) {
                    continue;
                }

                options.push_back({node, option});
            }
        }
    }

 private:
    RRTNode* root;
    RRTNode* current_head;
    std::unordered_map<RRTPoint, RRTNode*, PointHashFunction> node_map{};
    std::unordered_map<std::pair<RRTNode*, RRTNode*>, RRTEdge, EdgeHashFunction> edge_map{};

    Environment airspace;
    Dubins dubins;

    /**
     * Gets the nearest node to a given RRTPoint NOT USED AT THE MOMENT
     *
     * @param point     ==> the point to find the nearest node to
     * @return          ==> the nearest node to the point
     */
    std::pair<RRTNode*, double> getNearestNode(const XYZCoord& point) const;

    /**
     * RRTStar Recursive
     *  (RECURSIVE HELPER)
     * Rewires the tree by finding paths that are more efficintly routed through
     * the sample. Only searches for nodes a specific radius around the sample
     * to reduce computational expense
     *
     * @param current_node   ==> current node (DFS)
     * @param sample         ==> sampled point
     * @param search_radius  ==> the radius to search for nodes to rewire
     */
    void RRTStarRecursive(RRTNode* current_node, RRTNode* sample, double rewire_radius);

    /**
     * After rewire edge, it goes down the tree and reassigns the cost of the
     * nodes
     *
     * @param changed_node the node that has been changed
     */
    void reassignCosts(RRTNode* changed_node);

    /**
     *  Recurses down the tree to reassign the costs of the nodes
     * (RECURSIVE HELPER)
     *
     * @param parent        ==> the parent node
     * @param node          ==> the current node
     * @param path_cost     ==> the cost of the path to the current node
     */
    void reassignCostsRecursive(RRTNode* parent, RRTNode* current_node, double path_cost);
};

#endif  // INCLUDE_PATHING_TREE_HPP_
