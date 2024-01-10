#ifndef PATHING_TREE_HPP_
#define PATHING_TREE_HPP_

#include "utilities/datatypes.hpp"
#include <vector>
#include <unordered_map>

class RRTNode;
typedef std::vector<RRTNode*>  RRTNodeList;

struct RRTPoint {
    RRTPoint(XYZCoord point, double psi);
    /*
    *  Equality overload method for RRTPoint
    */
    bool operator== (const RRTPoint &otherPoint) const;

    double distanceTo(const RRTPoint &otherPoint) const;

    XYZCoord point;
    double psi;

};

class RRTNode {

    public:
        RRTNode(RRTPoint point, double cost);
        RRTNode(RRTPoint point, double cost, RRTNodeList reachable);
        
       /*
       *  Equality overload method for RRTNode object
       */
        bool operator== (const RRTNode &otherNode) const;

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

//Hash functions for the tree's member variables
class PointHashFunction {
    public:
        /* 
        *  Hashes RRTPoint using the Cantor Pairing Function. 
        *  Used to add elements to unordered_map nodeMap in RRTTree.
        */
        std::size_t operator()(const RRTPoint &point) const;
};

class EdgeHashFunction {
    public:
        /* 
        *  Hashes RRTNode* pair using the Cantor Pairing Function and RRTPoint hash function. 
        *  Used to add elements to unordered_map edgeMap in RRTTree.
        */
        std::size_t operator()(const std::pair<RRTNode*, RRTNode*> &nodePair) const;
};

class RRTEdge {
    public:
        RRTEdge(RRTNode* from, RRTNode* to, std::vector<XYZCoord> path, double cost);

        /*
        *  Equality overload method for RRTEdge comparison
        */
        bool operator == (const RRTEdge &otherEdge) const;

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
        const std::vector<XYZCoord>& getPath();

        /*
        *  Set the path vector associated with this edge
        */
        void setPath(std::vector<XYZCoord> path);

    private:
        RRTNode* from;
        RRTNode* to;
        double cost;
        std::vector<XYZCoord> path{};
};

class RRTTree {
    public:
        /*
        *  Add a node to the RRTTree. 
        *  If adding the first node to the tree, connectTo can be anything.
        */
        void addNode(RRTNode* connectTo, RRTNode* newNode, std::vector<XYZCoord> path, double cost);

        /*
        * Delete an edge between 'from' and 'toPrev', and create a new edge 
        * between 'from' to 'toNew'. Add 'toNew' to the nodeMap, and delete 'toPrev'.
        */
        void rewireEdge(RRTNode* from, RRTNode* toPrev, RRTNode* toNew, std::vector<XYZCoord> path, double cost);

        /*
        *  Returns a pointer to the node in the tree corresponding to the RRTPoint.
        *  If the node doesn't exist in the tree, returns nullptr.
        */
        RRTNode* getNode(RRTPoint point);

        /*
        *  Returns a pointer to the edge in the tree corresponding to the point pair.
        *  If the edge doesn't exist in the tree, returns nullptr.
        */
        RRTEdge* getEdge(RRTPoint from, RRTPoint to);

    private:
        std::unordered_map<RRTPoint, RRTNode*, PointHashFunction> nodeMap{};
        std::unordered_map<std::pair<RRTNode*, RRTNode*>, RRTEdge, EdgeHashFunction> edgeMap{};
};

#endif // PATHING_TREE_HPP_