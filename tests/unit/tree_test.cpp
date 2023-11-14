#include "../include/pathing/tree.hpp"
#include "../include/utilities/datatypes.hpp"
#include <gtest/gtest.h>

TEST(SimpleTreeTest, addNodeTest) {
    RRTPoint point1 = RRTPoint(XYZCoord(1, 2, 0), 0);
    RRTPoint point2 = RRTPoint(XYZCoord(0, 2, 0), 0);
    RRTNode a = RRTNode(point1, 10);
    RRTNode b = RRTNode(point2, 10);
    std::vector<RRTPoint> path = {RRTPoint(XYZCoord(0.5, 2, 0), 0), RRTPoint(XYZCoord(0.25, 2, 0), 0)};
    double edgeCost = 1;
    RRTEdge edge = RRTEdge(&a, &b, path, edgeCost);
    RRTTree simpleTree = RRTTree();

    simpleTree.addNode(&a, &a, std::vector<RRTPoint>(), 0);
    simpleTree.addNode(&a, &b, path, edgeCost);

    EXPECT_TRUE(a.getReachable()->size() > 0);
    
    EXPECT_TRUE(a.getReachable()->back() == &b);
    EXPECT_TRUE(b.getReachable()->back() == &a);

    EXPECT_TRUE(simpleTree.getNode(point1) != nullptr);
    EXPECT_TRUE(simpleTree.getNode(point2) != nullptr);

    EXPECT_TRUE(simpleTree.getEdge(point1, point2) != nullptr);
}

TEST(SimpleTreeTest, getNodeTest) {
    RRTPoint point1 = RRTPoint(XYZCoord(1, 2, 0), 0);
    RRTPoint point2 = RRTPoint(XYZCoord(0, 2, 0), 0);
    RRTNode a = RRTNode(point1, 10);
    RRTNode b = RRTNode(point2, 10);
    std::vector<RRTPoint> path = {RRTPoint(XYZCoord(0.5, 2, 0), 0), RRTPoint(XYZCoord(0.25, 2, 0), 0)};
    double edgeCost = 1;
    RRTEdge edge = RRTEdge(&a, &b, path, edgeCost);
    RRTTree simpleTree = RRTTree();
    simpleTree.addNode(&a, &a, std::vector<RRTPoint>(), 0);
    simpleTree.addNode(&a, &b, path, edgeCost);

    EXPECT_TRUE(simpleTree.getNode(point1) != nullptr);
    EXPECT_TRUE(*(simpleTree.getNode(point1)) == a);

    EXPECT_TRUE(simpleTree.getNode(point2) != nullptr);
    EXPECT_TRUE(*(simpleTree.getNode(point2)) == b);
}

TEST(SimpleTreeTest, getEdgeTest) {
    RRTPoint point1 = RRTPoint(XYZCoord(1, 2, 0), 0);
    RRTPoint point2 = RRTPoint(XYZCoord(0, 2, 0), 0);
    RRTNode a = RRTNode(point1, 10);
    RRTNode b = RRTNode(point2, 10);
    std::vector<RRTPoint> path = {RRTPoint(XYZCoord(0.5, 2, 0), 0), RRTPoint(XYZCoord(0.25, 2, 0), 0)};
    double edgeCost = 1;
    RRTEdge edge = RRTEdge(&a, &b, path, edgeCost);
    RRTTree simpleTree = RRTTree();
    simpleTree.addNode(&a, &a, std::vector<RRTPoint>(), 0);
    simpleTree.addNode(&a, &b, path, edgeCost);

    EXPECT_TRUE(simpleTree.getEdge(point1, point2) != nullptr);
    EXPECT_TRUE(*(simpleTree.getEdge(point1, point2)) == edge);
}

TEST(SimpleTreeTest, rewireEdgeTest) {
    RRTPoint point1 = RRTPoint(XYZCoord(1, 2, 0), 0);
    RRTPoint point2 = RRTPoint(XYZCoord(0, 2, 0), 0);
    RRTNode a = RRTNode(point1, 10);
    RRTNode b = RRTNode(point2, 10);
    std::vector<RRTPoint> path = {RRTPoint(XYZCoord(0.5, 2, 0), 0), RRTPoint(XYZCoord(0.25, 2, 0), 0)};
    double edgeCost = 1;

    RRTTree simpleTree = RRTTree();

    simpleTree.addNode(&a, &a, std::vector<RRTPoint>(), 0);
    simpleTree.addNode(&a, &b, path, edgeCost);

    RRTPoint point3 = RRTPoint(XYZCoord(3,2,0), 0);
    RRTNode c = RRTNode(point3, 10);
    RRTEdge edge = RRTEdge(&a, &c, path, edgeCost);

    simpleTree.rewireEdge(&a, &b, &c, path, edgeCost);

    EXPECT_TRUE(a.getReachable()->back() == &c);

    EXPECT_TRUE(simpleTree.getNode(point2) == nullptr);
    EXPECT_TRUE(simpleTree.getNode(point3) != nullptr);
    EXPECT_TRUE(simpleTree.getNode(point3) == &c);

    EXPECT_TRUE(simpleTree.getEdge(point1, point3) != nullptr);
    EXPECT_TRUE(*(simpleTree.getEdge(point1, point3)) == edge);
}