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
    RRTNode(RRTPoint point, double cost);
    RRTNode(RRTPoint point, double cost, RRTNodeList reachable);
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
    void setReachable(RRTNodeList reachable);

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
    RRTEdge(RRTNode* from, RRTNode* to, std::vector<Vector> path, double cost);

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
    bool addNode(RRTNode* anchor_node, RRTPoint& new_point, const RRTOption& option);  // NOLINT

    /*
     * Delete an edge between 'from' and 'toPrev', and create a new edge
     * between 'from' to 'toNew'. Add 'toNew' to the node_map, and delete
     * 'toPrev'.
     */
    void rewireEdge(RRTNode* current_point, RRTNode* previous_connection, RRTNode* new_connection,
                    std::vector<Vector> path, double cost);

    /*
     *  Returns a pointer to the node in the tree corresponding to the RRTPoint.
     *  If the node doesn't exist in the tree, returns nullptr.
     */
    RRTNode* getNode(RRTPoint point);

    /*
     *  Returns a pointer to the edge in the tree corresponding to the point
     * pair. If the edge doesn't exist in the tree, returns nullptr.
     */
    RRTEdge* getEdge(RRTPoint from, RRTPoint to);

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
     * returns the Environment object
     *
     * @return Environment object
     */
    Environment getAirspace() const;

    /**
     * Gets a random point in the environment
     *
     * @param search_radius radius to search for point
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
    std::vector<std::pair<RRTNode*, RRTOption>> pathingOptions(const RRTPoint& end,
                                                               int quantity_options = 10) {
        // fills the options list with valid values
        std::vector<std::pair<RRTNode*, RRTOption>> options;
        fillOptions(&options, root, end);

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

    /**
     * traverses the tree, and puts in all RRTOptions from dubins into a list
     * (DFS)
     *
     * @param options   ==> The list of options that is meant to be filled
     * @param node      ==> current node that will be traversed (DFS)
     */
    void fillOptions(std::vector<std::pair<RRTNode*, RRTOption>>* options, RRTNode* node,
                     const RRTPoint& end) {
        /*
           TODO - try to limit the scope of the search to prevent too many calls to dubins
        */
        if (node == nullptr) {
            return;
        }

        std::vector<RRTOption> local_options = dubins.allOptions(node->getPoint(), end);

        for (const RRTOption& option : local_options) {
            if (option.length != std::numeric_limits<double>::infinity()) {
                options->emplace_back(std::pair<RRTNode*, RRTOption>{node, option});
            }
        }

        for (RRTNode* child : node->getReachable()) {
            fillOptions(options, child, end);
        }
    }

    /**
     * Retreives the path to goal after the goal has been found
     *
     * TODO - find the goal without cache
     *
     */
    std::vector<XYZCoord> getPathToGoal(bool without_cache = false) {
        if (without_cache) {
            return {};  // TODO
        }

        if (!airspace.isGoalFound() || path_to_goal.size() == 0) {
            return {};  // should probably throw an error
        }

        return path_to_goal;
    }

    //  private:
    RRTNode* root;
    std::unordered_map<RRTPoint, RRTNode*, PointHashFunction> node_map{};
    std::unordered_map<std::pair<RRTNode*, RRTNode*>, RRTEdge, EdgeHashFunction> edge_map{};
    std::unordered_set<RRTNode*> leaf_nodes{};
    std::vector<XYZCoord> path_to_goal = {};

    Environment airspace;
    Dubins dubins;

    double distance_to_goal;

    /**
     * When the goal is found, recursively add the path constructed
     */
    void retreivePathByNode(RRTNode* node, RRTNode* parent);
};

#endif  // INCLUDE_PATHING_TREE_HPP_
