#ifndef INCLUDE_PATHING_TREE_HPP_
#define INCLUDE_PATHING_TREE_HPP_

#include <algorithm>
#include <limits>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/obc_config.hpp"
#include "utilities/rng.hpp"

class RRTNode;
typedef std::vector<std::shared_ptr<RRTNode>> RRTNodeList;
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
    bool operator==(const RRTNode& other_node) const;

    /*
     *  Get the RRTPoint associated with this RRTNode object
     */
    RRTPoint& getPoint();

    /*
     *  Set the reachable (neighbors) list for this RRTNode object
     */
    void setReachable(const RRTNodeList& reachable);

    /*
     *  Add a new node to the end of this node's reachable list.
     */
    void addReachable(std::shared_ptr<RRTNode> new_node);

    /*
     *  Remove a specific node from this node's reachable list.
     */
    void removeReachable(std::shared_ptr<RRTNode> old_node);

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
    std::vector<XYZCoord> path{};
};

class RRTTree {
 public:
    RRTTree(RRTPoint root_point, Environment airspace, Dubins dubins);
    ~RRTTree();

    /**
     * Generates node without adding it to the tree
     */
    std::shared_ptr<RRTNode> generateNode(std::shared_ptr<RRTNode> anchor_node,
                                          const RRTPoint& new_point,
                                          const RRTOption& option) const;

    /**
     * Adds a node to the tree
     *
     * @param anchor_node   ==> the node to connect to
     * @param new_node      ==> the node to add
     */
    bool addNode(std::shared_ptr<RRTNode> anchor_node, std::shared_ptr<RRTNode> new_node);

    /*
     *  Add a node to the RRTTree.
     *  If adding the first node to the tree, connectTo can be anything.
     */
    std::shared_ptr<RRTNode> addSample(std::shared_ptr<RRTNode> anchor_node,
                                       const RRTPoint& new_point,
                                       const RRTOption& option);

    /**
     * Returns a pointer to the root node
     *
     * @return std::shared_ptr<RRTNode> pointer to root node
     */
    std::shared_ptr<RRTNode> getRoot() const;

    /**
     * Get goal point
     *
     * @return RRTPoint goal point
     */
    const XYZCoord& getGoal() const;

    /**
     * Get goal point
     *
     * @return RRTPoint goal point
     */
    const XYZCoord& getGoal(int index) const;

    /**
     * returns the Environment object
     *
     * @return Environment object
     */
    const Environment& getAirspace() const;

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
    std::vector<std::pair<std::shared_ptr<RRTNode>, RRTOption>> pathingOptions(
        const RRTPoint& end, PointFetchMethod::Enum path_option = PointFetchMethod::Enum::NONE,
        int quantity_options = MAX_DUBINS_OPTIONS_TO_PARSE) const;

    /** DOES RRT* for the program
     *
     * @param sample          ==> the point to used as the base
     * @param rewire_radius   ==> the radius to search for nodes to rewire
     */
    void RRTStar(std::shared_ptr<RRTNode> sample, double rewire_radius);

    /**
     * Changes the currentHead to the given goal
     *
     * @param goal ==> the goal to change the currentHead to
     */
    void setCurrentHead(std::shared_ptr<RRTNode> goal);

    /**
     * Rewires an edge from an old path to a new path.
     * preserves ALL elements of the tree (i.e. NO elements are removed).
     *
     * @param current_point         ==> the current/end point to be rewired
     * @param previous_parent       ==> the previous parent to the current point
     * @param new_parent            ==> the new parent to the current point
     * @param path                  ==> the new path new_parrent --> current_point
     * @param path_cost             ==> the cost of the new path
     */
    void rewireEdge(std::shared_ptr<RRTNode> current_point,
                    std::shared_ptr<RRTNode> previous_parent,
                    std::shared_ptr<RRTNode> new_parent,
                    const std::vector<Vector>& path, double path_cost);

    /**
     * Gets K random nodes from the tree, starting at the current head
     *
     * @param k ==> the number of nodes to get
     * @return  ==> list of k random nodes (unordered)
     */
    std::vector<std::shared_ptr<RRTNode>> getKRandomNodes(int k) const;

    /**
     * __Recursive Helper__
     * Gets K random nodes from the tree, starting at the current head
     *
     * @param nodes         ==> the list (reference) of nodes to add to
     * @param current_node  ==> the current node that is being accessed
     * @param k             ==> the number of nodes to get
     * @param chance        ==> the chance to add the current node to the list
     */
    void getKRandomNodesRecursive(std::vector<std::shared_ptr<RRTNode>>& nodes,
                                  std::shared_ptr<RRTNode> current_node,
                                  double chance) const;

    /**
     * Gets the k closest nodes to a given point
     *
     * @param sample    ==> the point to find the closest nodes to
     * @param k         ==> the number of nodes to get
     * @return          ==> list (ordered) of k closest nodes
     */
    std::vector<std::shared_ptr<RRTNode>> getKClosestNodes(const RRTPoint& sample, int k) const;

    /**
     * __Recursive Helper__
     * Gets the k closest nodes to a given point
     *
     * @param nodes_by_distance ==> the list (reference) of {distance, node} to add to
     * @param sample            ==> the point to find the closest nodes to
     * @param current_node      ==> the current node that is being accessed
     */
    void getKClosestNodesRecursive(std::vector<std::pair<double,
                                   std::shared_ptr<RRTNode>>>& nodes_by_distance,
                                   const RRTPoint& sample,
                                   std::shared_ptr<RRTNode> current_node) const;

    /**
     * Fills in a list of options from an existing list of nodes
     *
     * @param options   ==> the list of options to fill
     * @param nodes     ==> the list of nodes to parse
     * @param sample    ==> the end point that the options will be connected to
     */
    void fillOptionsNodes(std::vector<std::pair<std::shared_ptr<RRTNode>,
                          RRTOption>>& options,
                          const std::vector<std::shared_ptr<RRTNode>>& nodes,
                          const RRTPoint& sample) const;

    /**
     * Returns the segment of path from the given node to the current head
     *
     * @param node  ==> the node to start the path from
     * @return      ==> the path from the node to the current head
     */
    std::vector<XYZCoord> getPathSegment(std::shared_ptr<RRTNode> node) const;

    /**
     * Returns the start RRTPoint
     *
     * @return RRTPoint start point
     */
    RRTPoint& getStart() const;

 private:
    std::shared_ptr<RRTNode> root;
    std::shared_ptr<RRTNode> current_head;
    Environment airspace;
    Dubins dubins;
    int tree_size;

    /**
     * Helper that deletes the tree
     *
     * @param node ==> the root of the tree to delete
     */
    void deleteTree(std::shared_ptr<RRTNode> node);

    /**
     * traverses the tree, and puts in all RRTOptions from dubins into a list
     * (DFS)
     *
     * @param options   ==> The list of options that is meant to be filled
     * @param node      ==> current node that will be traversed (DFS)
     * @param sample    ==> the end point that the options will be connected to
     */
    void fillOptions(std::vector<std::pair<std::shared_ptr<RRTNode>, RRTOption>>& options,
                     std::shared_ptr<RRTNode> node,
                     const RRTPoint& sample) const;

    /**
     * Gets the nearest node to a given RRTPoint NOT USED AT THE MOMENT
     *
     * @param point     ==> the point to find the nearest node to
     * @return          ==> the nearest node to the point
     */
    // std::pair<std::shared_ptr<RRTNode>, double> getNearestNode(const XYZCoord& point) const;

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
    void RRTStarRecursive(std::shared_ptr<RRTNode> current_node,
                          std::shared_ptr<RRTNode> sample,
                          double rewire_radius);

    /**
     * After rewire edge, it goes down the tree and reassigns the cost of the
     * nodes
     *
     * @param changed_node the node that has been changed
     */
    void reassignCosts(std::shared_ptr<RRTNode> changed_node);

    /**
     *  Recurses down the tree to reassign the costs of the nodes
     * (RECURSIVE HELPER)
     *
     * @param parent        ==> the parent node
     * @param node          ==> the current node
     * @param path_cost     ==> the cost of the path to the current node
     */
    void reassignCostsRecursive(std::shared_ptr<RRTNode> parent,
                                std::shared_ptr<RRTNode> current_node,
                                double cost_difference);

    /**
     * Finds the sequence of nodes from current_head to the target node using BFS
     *
     * @param target_node   ==> the node to find the path to
     * @return              ==> vector of nodes from current_head to target_node
     */
    RRTNodeList findPathToNode(std::shared_ptr<RRTNode> target_node) const;

    /**
     * Constructs the coordinate path from a sequence of nodes
     *
     * @param nodes         ==> sequence of nodes
     * @return              ==> vector of coordinates representing the path
     */
    std::vector<XYZCoord> buildPathFromNodes(
                           const std::vector<std::shared_ptr<RRTNode>>& nodes) const;
};

#endif  // INCLUDE_PATHING_TREE_HPP_
