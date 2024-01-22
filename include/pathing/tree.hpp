#ifndef INCLUDE_PATHING_TREE_HPP_
#define INCLUDE_PATHING_TREE_HPP_

#include <unordered_map>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "utilities/datatypes.hpp"

class RRTNode;
typedef std::vector<RRTNode*> RRTNodeList;
typedef XYZCoord Vector;


class RRTNode {
 public:
    RRTNode(RRTPoint point, double cost);
    RRTNode(RRTPoint point, double cost, RRTNodeList reachable);

    /*
     *  Equality overload method for RRTNode object
     */
    bool operator==(const RRTNode& otherNode) const;

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
    void addReachable(RRTNode* newNode);

    /*
     *  Remove a specific node from this node's reachable list.
     *  Set the removed node's parent pointer to be null.
     */
    void removeReachable(RRTNode* oldNode);

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
    void setCost(double newCost);

 private:
    RRTPoint point;
    RRTNodeList reachable{};
    double cost;
    RRTNode* parent{};
};

// Hash functions for the tree's member variables
class PointHashFunction {
 public:
    /*
     *  Hashes RRTPoint using the Cantor Pairing Function.
     *  Used to add elements to unordered_map nodeMap in RRTTree.
     */
    std::size_t operator()(const RRTPoint& point) const;
};

class EdgeHashFunction {
 public:
    /*
     *  Hashes RRTNode* pair using the Cantor Pairing Function and RRTPoint hash
     * function. Used to add elements to unordered_map edgeMap in RRTTree.
     */
    std::size_t operator()(const std::pair<RRTNode*, RRTNode*>& nodePair) const;
};

class RRTEdge {
 public:
    RRTEdge(RRTNode* from, RRTNode* to, std::vector<Vector> path, double cost);

    /*
     *  Equality overload method for RRTEdge comparison
     */
    bool operator==(const RRTEdge& otherEdge) const;

    /*
     *  Set the cost of this edge
     */
    void setCost(double newCost);

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
    RRTTree(RRTPoint rootPoint, RRTPoint goalPoint);
    ~RRTTree();
    /*
     *  Add a node to the RRTTree.
     *  If adding the first node to the tree, connectTo can be anything.
     */
    void addNode(RRTNode* connectTo, RRTNode* newNode, std::vector<Vector> path, double cost);

    /*
     * Delete an edge between 'from' and 'toPrev', and create a new edge
     * between 'from' to 'toNew'. Add 'toNew' to the nodeMap, and delete
     * 'toPrev'.
     */
    void rewireEdge(RRTNode* from, RRTNode* toPrev, RRTNode* toNew, std::vector<Vector> path,
                    double cost);

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

 private:
    RRTNode* root;
    RRTPoint goalPoint;
    std::unordered_map<RRTPoint, RRTNode*, PointHashFunction> nodeMap{};
    std::unordered_map<std::pair<RRTNode*, RRTNode*>, RRTEdge, EdgeHashFunction> edgeMap{};

    Polygon bounds;

    Dubins dubins;

    double tolerance_to_goal;
    double distance_to_goal;

    bool found_goal;
};

#endif  // INCLUDE_PATHING_TREE_HPP_
