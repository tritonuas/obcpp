#ifndef PATHING_TREE_HPP_
#define PATHING_TREE_HPP_

#include <vector>
#include <unordered_map>
class RRTNode;
typedef std::vector<RRTNode*>  RRTNodeList;

struct RRTPoint {
    RRTPoint(double x, double y, double z, double psi);

    double x;
    double y;
    double z;
    double psi;

};

class RRTNode {

    public:
        RRTNode(RRTPoint point);
        RRTNode(RRTPoint point, RRTNodeList reachable);
        
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
        RRTEdge(RRTNode* from, RRTNode* to);
        RRTEdge(RRTNode* from, RRTNode* to, std::vector<RRTPoint> path);
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
        void addNode(RRTNode* connectTo, RRTNode* newNode);
        void rewireEdge(RRTNode* from, RRTNode* toPrev, RRTNode* toNew);
        RRTNode* getNode(RRTPoint point);
        RRTEdge* getEdge(RRTPoint from, RRTPoint to);

    private:
        std::unordered_map<RRTPoint, RRTNode*> nodeMap;
        std::unordered_map<std::pair<RRTNode*, RRTNode*>, RRTEdge> edgeMap;
};

#endif // PATHING_TREE_HPP_