#ifndef INCLUDE_PATHING_TREE_HPP_
#define INCLUDE_PATHING_TREE_HPP_

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <memory>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/obc_config.hpp"
#include "utilities/rng.hpp"

class RRTNode;
typedef std::shared_ptr<RRTNode> RRTNodePtr;

class RRTNode {
 public:
    RRTNode(const RRTPoint& point, double cost, double path_length,
            const std::vector<XYZCoord> path);
    RRTNode(const RRTPoint& point, double cost, double path_length,
            const std::vector<XYZCoord> path, std::vector<RRTNodePtr> reachable);

    /**
     * Destructor for RRTNode object
     *
     * Removes the subtree below it (recursively)
     */
    ~RRTNode();

    /*
     *  Equality overload method for RRTNode object
     */
    // bool operator==(const RRTNode& other_node) const;

    /*
     *  Get the RRTPoint associated with this RRTNode object
     */
    RRTPoint& getPoint();

    /*
     *  Set the reachable (neighbors) list for this RRTNode object
     */
    void setReachable(const std::vector<RRTNodePtr>& reachable);

    /*
     *  Add a new node to the end of this node's reachable list.
     *  Set the new node's parent to be this node.
     */
    void addReachable(RRTNodePtr new_node);

    /*
     *  Remove a specific node from this node's reachable list.
     *  Set the removed node's parent pointer to be null.
     */
    void removeReachable(RRTNodePtr old_node);

    /*
     *  Return a reference to this node's reachable list
     */
    const std::vector<RRTNodePtr>& getReachable();

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
    std::vector<RRTNodePtr> reachable{};
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
    RRTNodePtr generateNode(RRTNodePtr anchor_node, const RRTPoint& new_point,
                          const RRTOption& option) const;

    /**
     * Adds a node to the tree
     *
     * @param anchor_node   ==> the node to connect to
     * @param new_node      ==> the node to add
     */
    bool addNode(RRTNodePtr anchor_node, RRTNodePtr new_node);

    /*
     *  Add a node to the RRTTree.
     *  If adding the first node to the tree, connectTo can be anything.
     */
    RRTNodePtr addSample(RRTNodePtr anchor_node, const RRTPoint& new_point, const RRTOption& option);

    /**
     * Returns a pointer to the root node
     *
     * @return RRTNodePtr pointer to root node
     */
    RRTNodePtr getRoot() const;

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

    bool validatePath(const std::vector<XYZCoord>& path, const RRTOption& options) const;

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
    std::vector<std::pair<RRTNodePtr, RRTOption>> pathingOptions(
        const RRTPoint& end, PointFetchMethod::Enum path_option = PointFetchMethod::Enum::NONE,
        int quantity_options = MAX_DUBINS_OPTIONS_TO_PARSE) const;

    /**
     * Changes the currentHead to the given goal
     *
     * @param goal ==> the goal to change the currentHead to
     */
    void setCurrentHead(RRTNodePtr goal);

    //  /**
    //   *  _____| UNUSED |_____
    //   * Returns a path to the goal from the root
    //   *
    //   * The currentHead must be the goal for this to properly
    //   * generate a complete path
    //   * @return  ==> list of 2-vectors to the goal region
    //   */
    //  std::vector<XYZCoord> getPathToGoal() const;

    /**
     * Fills in a list of options from an existing list of nodes
     *
     * @param options   ==> the list of options to fill
     * @param nodes     ==> the list of nodes to parse
     * @param sample    ==> the end point that the options will be connected to
     */
    void fillOptionsNodes(std::vector<std::pair<RRTNodePtr, RRTOption>>& options,
                          const std::vector<RRTNodePtr>& nodes, const RRTPoint& sample) const;

    /**
     * Returns the segment of path from the given node to the current head
     *
     * @param node  ==> the node to start the path from
     * @return      ==> the path from the node to the current head
     */
    std::vector<XYZCoord> getPathSegment(RRTNodePtr node) const;

    /**
     * Returns the start RRTPoint
     *
     * @return RRTPoint start point
     */
    RRTPoint& getStart() const;

 private:
    RRTNodePtr root;
    RRTNodePtr current_head;
    Environment airspace;
    Dubins dubins;
    int tree_size;

    /**
     * Helper that deletes the tree
     *
     * @param node ==> the root of the tree to delete
     */
    void deleteTree(RRTNodePtr node);

    /**
     * traverses the tree, and puts in all RRTOptions from dubins into a list
     * (DFS)
     *
     * @param options   ==> The list of options that is meant to be filled
     * @param node      ==> current node that will be traversed (DFS)
     * @param sample    ==> the end point that the options will be connected to
     */
    void fillOptions(std::vector<std::pair<RRTNodePtr, RRTOption>>& options, RRTNodePtr node,
                     const RRTPoint& sample) const;

    /**
     * Gets the nearest node to a given RRTPoint NOT USED AT THE MOMENT
     *
     * @param point     ==> the point to find the nearest node to
     * @return          ==> the nearest node to the point
     */
    // std::pair<RRTNodePtr, double> getNearestNode(const XYZCoord& point) const;
};

#endif  // INCLUDE_PATHING_TREE_HPP_
