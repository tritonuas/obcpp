#ifndef PATHING_TREE_HPP_
#define PATHING_TREE_HPP_

#include "../utilities/datatypes.hpp"
#include <vector>
#include <unordered_map>

class RRTNode;
typedef std::vector<RRTNode*>  RRTNodeList;

struct RRTPoint {
    RRTPoint(XYZCoord xyz, double psi);
    bool operator== (const RRTPoint &otherPoint) const;

    XYZCoord xyz;
    double psi;

};

class RRTNode {

    public:
        RRTNode(RRTPoint point, double cost);
        RRTNode(RRTPoint point, double cost, RRTNodeList reachable);
        
        bool operator== (const RRTNode &otherNode) const;
        RRTPoint getPoint();
        void setReachable(RRTNodeList reachable);
        void addReachable(RRTNode* newNode);
        RRTNodeList* getReachable();
        double getCost();
        void setCost(double newCost);

    private:
        RRTPoint point;
        RRTNodeList reachable;
        double cost;
        RRTNode* parent;
};

//Hash functions for the tree's member variables
//unsigned int hashPoint(const RRTPoint &point);
//unsigned int hashEdge(const std::pair<RRTNode*, RRTNode*> &nodePair);
class PointHashFunction {
    public:
        std::size_t operator()(const RRTPoint &point) const;
};

class EdgeHashFunction {
    public:
        std::size_t operator()(const std::pair<RRTNode*, RRTNode*> &nodePair) const;
};

class RRTEdge {
    public:
        RRTEdge(RRTNode* from, RRTNode* to, std::vector<RRTPoint> path, double cost);
        bool operator == (const RRTEdge &otherEdge) const;
        void setCost(double newCost);
        double getCost();
        std::vector<RRTPoint> getPath();
        void setPath(std::vector<RRTPoint> path);

    private:
        RRTNode* from;
        RRTNode* to;
        double cost;
        std::vector<RRTPoint> path;
};

class RRTTree {
    public:
        void addNode(RRTNode* connectTo, RRTNode* newNode, std::vector<RRTPoint> path, double cost);
        void rewireEdge(RRTNode* from, RRTNode* toPrev, RRTNode* toNew, std::vector<RRTPoint> path, double cost);
        RRTNode* getNode(RRTPoint point);
        RRTEdge* getEdge(RRTPoint from, RRTPoint to);

    //private:
        std::unordered_map<RRTPoint, RRTNode*, PointHashFunction> nodeMap;
        std::unordered_map<std::pair<RRTNode*, RRTNode*>, RRTEdge, EdgeHashFunction> edgeMap;
};

#endif // PATHING_TREE_HPP_