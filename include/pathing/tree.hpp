#ifndef INCLUDE_PATHING_TREE_HPP_
#define INCLUDE_PATHING_TREE_HPP_

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
    RRTNode(const RRTPoint& point, double cost);
    RRTNode(const RRTPoint& point, double cost, RRTNodeList& reachable);  // NOLINT
    //  ~RRTNode();

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
    RRTNode* addNode(RRTNode* anchor_node, const RRTPoint& new_point, const RRTOption& option,
                     int goal_index);

    /*

     */
    void rewireEdge(RRTNode* current_point, RRTNode* previous_connection, RRTNode* new_connection,
                    const std::vector<Vector>& path, double cost);

    /*
     *  Returns a pointer to the node in the tree corresponding to the RRTPoint.
     *  If the node doesn't exist in the tree, returns nullptr.
     */
    RRTNode* getNode(const RRTPoint& point);

    /*
     *  Returns a pointer to the edge in the tree corresponding to the point
     * pair. If the edge doesn't exist in the tree, returns nullptr.
     */
    RRTEdge* getEdge(const RRTPoint& from, const RRTPoint& to);

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
    RRTPoint getGoal() const;

    /**
     * Get goal point
     *
     * @return RRTPoint goal point
     */
    RRTPoint getGoal(int index) const;

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
    RRTPoint getRandomPoint(double search_radius, bool use_goal);

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
    std::vector<std::pair<RRTNode*, RRTOption>> pathingOptions(const RRTPoint& end,
                                                               int quantity_options = 10);

    /**
     * traverses the tree, and puts in all RRTOptions from dubins into a list
     * (DFS)
     *
     * @param options   ==> The list of options that is meant to be filled
     * @param node      ==> current node that will be traversed (DFS)
     */
    void fillOptions(std::vector<std::pair<RRTNode*, RRTOption>>* options, RRTNode* node,
                     const RRTPoint& end);

    /**
     * Retreives the path to goal after the goal has been found
     *
     * TODO - find the goal without cache
     *
     * @param without_cache ==> whether or not to use the first generated path when goal is found
     * @return              ==> the path to the goal
     */
    std::vector<XYZCoord> getPathToGoal(bool without_cache = false);

    /** DOES RRT* for the program
     *
     * @param sample          ==> the point to used as the base
     * @param rewire_radius   ==> the radius to search for nodes to rewire
     */
    void RRTStar(RRTNode* sample, double rewire_radius);

    /**
     * changes current_head to the goal node with the closest length
     * TODO - ensure that the path is the most efficient
     * @param goal_index the current_goal index to find the goal to
     */
    void changeCurrentHead(int goal_index) {
        // gets the nodes in the goal
        std::vector<RRTNode*> nodes = getNodesInGoal(goal_index);

        if (nodes.size() == 0) {
            return;
        }

        // finds the node with the shortest path
        RRTNode* goal = nodes[0];

        for (RRTNode* node : nodes) {
            if (node->getCost() < goal->getCost()) {
                goal = node;
            }
        }

        current_head = goal;
    }

 private:
    RRTNode* root;
    RRTNode* current_head;
    std::unordered_map<RRTPoint, RRTNode*, PointHashFunction> node_map{};
    std::unordered_map<std::pair<RRTNode*, RRTNode*>, RRTEdge, EdgeHashFunction> edge_map{};
    std::vector<XYZCoord> path_to_goal = {};

    Environment airspace;
    Dubins dubins;

    double distance_to_goal;  // not used at the moment

    /**
     * Returns all the nodes in the specified goal area recursively
     *
     * @param goal_index the index of the goal to search for
     */
    std::vector<RRTNode*> getNodesInGoal(int goal_index) {
        std::vector<RRTNode*> nodes;
        getNodesInGoalRecursive(root, goal_index, nodes);
        return nodes;
    }

    /**
     * Recursive helper for getNodesInGoal
     *
     * @param node the current node to search
     * @param goal_index the index of the goal to search for
     * @param nodes the list of nodes to add to
     */
    void getNodesInGoalRecursive(RRTNode* node, int goal_index, std::vector<RRTNode*>& nodes) {
        if (airspace.isPointInGoal(node->getPoint().coord, goal_index)) {
            nodes.push_back(node);
            return;
        }

        for (RRTNode* neighbor : node->getReachable()) {
            getNodesInGoalRecursive(neighbor, goal_index, nodes);
        }
    }

    /**
     * When the goal is found, recursively add the path constructed
     */
    void retreivePathByNode(RRTNode* node, RRTNode* parent);

    /**
     * Gets the nearest node to a given RRTPoint
     *
     * @param point     ==> the point to find the nearest node to
     * @return          ==> the nearest node to the point
     */
    RRTNode* getNearestNode(const RRTPoint& point);

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
     * Fills out an array of List<Node*> and path lengths to the goal using depth first search
     * TODO - test
     *
     * @param options   ==> list of options to fill
     * @param node      ==> current node to traverse
     * @param parent    ==> the parent of the current node
     * @param current_length ==> the current length of the path
     * @param current_path   ==> the current path being traced by DFS
     */
    void fillPathOptionsRecursive(std::vector<std::pair<std::vector<RRTNode*>, double>>& options,
                                  RRTNode* node, RRTNode* parent, double current_length,
                                  std::vector<RRTNode*>& current_path) {
        // parent should never be nullptr, but for safety
        if (node == nullptr || parent == nullptr) {
            return;
        }

        // find the edge between the parent and current node
        RRTEdge* edge = getEdge(parent->getPoint(), node->getPoint());

        if (edge == nullptr) {
            return;
        }

        double edge_cost = edge->getCost();

        current_path.push_back(node);

        if (airspace.isPointInGoal(node->getPoint().coord)) {
            // add the path to the goal (deep copy of current_path)
            options.push_back(std::make_pair(current_path, current_length + edge_cost));
            current_path.pop_back();
            return;
        }

        for (RRTNode* neighbor : node->getReachable()) {
            fillPathOptionsRecursive(options, neighbor, node, current_length + edge_cost,
                                     current_path);
        }

        current_path.pop_back();
    }
};

#endif  // INCLUDE_PATHING_TREE_HPP_
