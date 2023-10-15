#ifndef PATHING_TREE_HPP_
#define PATHING_TREE_HPP_

#include <vector>
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
    // TODO: Declare methods/fields
};

#endif // PATHING_TREE_HPP_