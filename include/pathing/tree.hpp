#ifndef PATHING_TREE_HPP_
#define PATHING_TREE_HPP_

#include "../utilities/datatypes.hpp"
#include <vector>
#include <unordered_map>
class RRTNode;
typedef std::vector<RRTNode*>  RRTNodeList;

struct RRTPoint {
    RRTPoint(XYZCoord xyz, double psi);

    XYZCoord xyz;
    double psi;

};

class RRTNode {

    public:
        RRTNode(RRTPoint point, double cost);
        RRTNode(RRTPoint point, double cost, RRTNodeList reachable);
        
        RRTPoint getPoint();
        void setReachable(RRTNodeList reachable);
        RRTNodeList getReachable();
        double getCost();
        void setCost(double newCost);

    private:
        RRTPoint point;
        RRTNodeList reachable;
        double cost;
        RRTNode* parent;
};

class RRTEdge {
    public:
        RRTEdge(RRTNode* from, RRTNode* to, std::vector<RRTPoint> path, double cost);
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

    private:
        std::unordered_map<RRTPoint, RRTNode> nodeMap;
        std::unordered_map<std::pair<RRTNode*, RRTNode*>, RRTEdge> edgeMap;
};

#endif // PATHING_TREE_HPP_