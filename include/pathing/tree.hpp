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
    RRTNode* addNode(RRTNode* anchor_node, const RRTPoint& new_point, const RRTOption& option);

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
    RRTPoint getRandomPoint(double search_radiusl);

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
     * Returns the currenthead of the tree
     */
    RRTNode* getCurrentHead() const { return current_head; }

    /**
     * Changes the currentHead to the given goal
     *
     * @param goal ==> the goal to change the currentHead to
     */
    void setCurrentHead(RRTNode* goal) {
        if (goal == nullptr || node_map[goal->getPoint()] == nullptr) {
            std::cout << "FAILURE: Goal is not in the tree\n" << std::endl;
            return;
        }
        current_head = goal;
    }

    //  private:
    RRTNode* root;
    RRTNode* current_head;
    std::unordered_map<RRTPoint, RRTNode*, PointHashFunction> node_map{};
    std::unordered_map<std::pair<RRTNode*, RRTNode*>, RRTEdge, EdgeHashFunction> edge_map{};
    std::vector<XYZCoord> path_to_goal = {};

    Environment airspace;
    Dubins dubins;

    double distance_to_goal;  // not used at the moment

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
     * After rewire edge, it goes down the tree and reassigns the cost of the
     * nodes
     *
     * @param changed_node the node that has been changed
     */
    void reassignCosts(RRTNode* changed_node) {
        if (changed_node == nullptr) {
            return;
        }

        for (RRTNode* child : changed_node->getReachable()) {
            reassignCostsRecursive(changed_node, child, changed_node->getCost());
        }
    }

    void reassignCostsRecursive(RRTNode* parent, RRTNode* node, double path_cost) {
        if (node == nullptr) {
            return;
        }

        // get edge cost
        double edge_cost = getEdge(parent->getPoint(), node->getPoint())->getCost();

        node->setCost(path_cost + edge_cost);
        for (RRTNode* neighbor : node->getReachable()) {
            reassignCostsRecursive(node, neighbor, node->getCost());
        }
    }
};

#endif  // INCLUDE_PATHING_TREE_HPP_
