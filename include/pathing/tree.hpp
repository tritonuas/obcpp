#ifndef INCLUDE_PATHING_TREE_HPP_
#define INCLUDE_PATHING_TREE_HPP_

#include <algorithm>
#include <limits>
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

class RRTNode {
 public:
    RRTNode(const RRTPoint& point, double cost, double path_length,
            const std::vector<XYZCoord> path);
    RRTNode(const RRTPoint& point, double cost, double path_length,
            const std::vector<XYZCoord> path, RRTNodeList reachable);

    /*
     *  Equality overload method for RRTNode object
     */
    // bool operator==(const RRTNode& other_node) const;

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

    /**
     * Get the path associated with this node
     *
     * @return std::vector<XYZCoord> path
     */
    const std::vector<XYZCoord>& getPath() const;

    /**
     * Set the path associated with this node
     *
     * @param path std::vector<XYZCoord> path
     */
    void setPath(const std::vector<XYZCoord>& path);

    /**
     * Get the path length associated with this node
     *
     * @return double path length
     */
    double getPathLength() const;

    /**
     * Set the path length associated with this node
     *
     * @param path_length double path length
     */
    void setPathLength(double path_length);

 private:
    RRTPoint point;
    RRTNodeList reachable{};
    double cost;
    double path_length;
    RRTNode* parent{};
    std::vector<XYZCoord> path{};
};

class RRTTree {
 public:
    RRTTree(RRTPoint root_point, Environment airspace, Dubins dubins);
    // ~RRTTree();

    /**
     * Generates node without adding it to the tree
     */
    RRTNode* generateNode(RRTNode* anchor_node, const RRTPoint& new_point,
                          const RRTOption& option) const;

    bool addNode(RRTNode* anchor_node, RRTNode* new_node);

    /*
     *  Add a node to the RRTTree.
     *  If adding the first node to the tree, connectTo can be anything.
     */
    RRTNode* addSample(RRTNode* anchor_node, const RRTPoint& new_point, const RRTOption& option);

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

    bool validatePath(const std::vector<Vector>& path, const RRTOption& options) const;

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
        const RRTPoint& end, PATH_OPTIONS path_option = PATH_OPTIONS::NONE,
        int quantity_options = MAX_DUBINS_OPTIONS_TO_PARSE) const;

    /** DOES RRT* for the program
     *
     * @param sample          ==> the point to used as the base
     * @param rewire_radius   ==> the radius to search for nodes to rewire
     */
    void RRTStar(RRTNode* sample, double rewire_radius);

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

    /*

     */
    void rewireEdge(RRTNode* current_point, RRTNode* previous_connection, RRTNode* new_connection,
                    const std::vector<Vector>& path, double path_cost);

    std::vector<RRTNode*> getKRandomNodes(const RRTPoint& sample, int k) const {
        std::vector<RRTNode*> nodes;
        double chance = 1.0 * k / tree_size;
        getKRandomNodesRecursive(nodes, current_head, k, chance);

        return nodes;
    }

    void getKRandomNodesRecursive(std::vector<RRTNode*>& nodes, RRTNode* current_node,
                                  int k, double chance) const {
        if (current_node == nullptr) {
            return;
        }

        if (random(0,1) < chance) {
            nodes.emplace_back(current_node);
        }

        for (RRTNode* node : current_node->getReachable()) {
            getKRandomNodesRecursive(nodes, node, k, chance);
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

        // Euclidian distance is weighted much more heavily
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
    Environment airspace;
    Dubins dubins;
    int tree_size;

    /**
     * Helper that deletes the tree
     *
     * @param node ==> the root of the tree to delete
     */
    void deleteTree(RRTNode* node);

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

    /**
     * Gets the nearest node to a given RRTPoint NOT USED AT THE MOMENT
     *
     * @param point     ==> the point to find the nearest node to
     * @return          ==> the nearest node to the point
     */
    // std::pair<RRTNode*, double> getNearestNode(const XYZCoord& point) const;

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
    void reassignCostsRecursive(RRTNode* parent, RRTNode* current_node, double cost_difference);
};

#endif  // INCLUDE_PATHING_TREE_HPP_
