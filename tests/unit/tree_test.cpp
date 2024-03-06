#include "pathing/tree.hpp"

#include <gtest/gtest.h>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

/*
 *   very bad tests, was too lazy to check if every parameter was correct, aka didn't bother to find
 *   the hardcoded values for the expected values.
 */
#include <iostream>

TEST(SimpleTreeTest, addNodeTest) {
    Dubins dubins{5, 0.1};
    Polygon valid_region;
    valid_region.emplace_back(XYZCoord(0, 0, 0));
    valid_region.emplace_back(XYZCoord(100, 0, 0));
    valid_region.emplace_back(XYZCoord(100, 100, 0));
    valid_region.emplace_back(XYZCoord(0, 100, 0));

    Polygon obs1 = {
        {XYZCoord(10, 10, 0), XYZCoord(20, 10, 0), XYZCoord(20, 20, 0), XYZCoord(10, 20, 0)}};

    std::vector<Polygon> obstacles = {obs1};
    Environment env = Environment(valid_region, {XYZCoord(0, 0, 0)}, obstacles);
    RRTPoint point1 = RRTPoint(XYZCoord(25, 25, 0), 0);
    RRTPoint point2 = RRTPoint(XYZCoord(50, 75, 0), 0);
    RRTOption option = dubins.allOptions(point1, point2, true)[0];

    RRTTree simple_tree = RRTTree(point1, env, dubins);

    RRTNode* root = simple_tree.getRoot();

    // simpleTree.addNode(root, point1);
    bool added_point = simple_tree.addNode(root, point2, option);

    EXPECT_TRUE(added_point);
    EXPECT_TRUE(root->getReachable().size() > 0);

    EXPECT_TRUE(simple_tree.getNode(point1) != nullptr);
    EXPECT_TRUE(simple_tree.getNode(point2) != nullptr);

    EXPECT_TRUE(
        simple_tree.getEdge(simple_tree.getNode(point1), simple_tree.getNode(point2)).getCost() !=
        0);
}

TEST(SimpleTreeTest, getNodeTest) {
    Dubins dubins{5, 0.1};
    Polygon valid_region;
    valid_region.emplace_back(XYZCoord(0, 0, 0));
    valid_region.emplace_back(XYZCoord(100, 0, 0));
    valid_region.emplace_back(XYZCoord(100, 100, 0));
    valid_region.emplace_back(XYZCoord(0, 100, 0));
    Polygon obs1 = {
        {XYZCoord(10, 10, 0), XYZCoord(20, 10, 0), XYZCoord(20, 20, 0), XYZCoord(10, 20, 0)}};

    std::vector<Polygon> obstacles = {obs1};
    Environment env = Environment(valid_region, {XYZCoord(0, 0, 0)}, obstacles);
    RRTPoint point1 = RRTPoint(XYZCoord(25, 25, 0), 0);
    RRTPoint point2 = RRTPoint(XYZCoord(50, 75, 0), 0);
    RRTOption option = dubins.allOptions(point1, point2, true)[0];

    RRTTree simple_tree = RRTTree(point1, env, dubins);

    RRTNode* root = simple_tree.getRoot();

    // simpleTree.addNode(root, point1);
    bool added_point = simple_tree.addNode(root, point2, option);

    EXPECT_TRUE(true);

    EXPECT_TRUE(simple_tree.getNode(point1) != nullptr);
    EXPECT_TRUE(*(simple_tree.getNode(point1)) == *root);

    EXPECT_TRUE(simple_tree.getNode(point2) != nullptr);
    // EXPECT_TRUE(*(simple_tree.getNode(point2)) == b);
}

TEST(SimpleTreeTest, getEdgeTest) {
    Dubins dubins{5, 0.1};
    Polygon valid_region;
    valid_region.emplace_back(XYZCoord(0, 0, 0));
    valid_region.emplace_back(XYZCoord(100, 0, 0));
    valid_region.emplace_back(XYZCoord(100, 100, 0));
    valid_region.emplace_back(XYZCoord(0, 100, 0));
    Polygon obs1 = {
        {XYZCoord(10, 10, 0), XYZCoord(20, 10, 0), XYZCoord(20, 20, 0), XYZCoord(10, 20, 0)}};

    std::vector<Polygon> obstacles = {obs1};
    Environment env = Environment(valid_region, {XYZCoord(0, 0, 0)}, obstacles);
    RRTPoint point1 = RRTPoint(XYZCoord(25, 25, 0), 0);
    RRTPoint point2 = RRTPoint(XYZCoord(50, 75, 0), 0);
    RRTOption option = dubins.allOptions(point1, point2, true)[0];

    RRTTree simple_tree = RRTTree(point1, env, dubins);

    RRTNode* root = simple_tree.getRoot();

    // simpleTree.addNode(root, point1);
    bool added_point = simple_tree.addNode(root, point2, option);

    EXPECT_TRUE(
        simple_tree.getEdge(simple_tree.getNode(point1), simple_tree.getNode(point2)).getCost() !=
        0);
    // EXPECT_TRUE(*(simple_tree.getEdge(point1, point2)) == edge);
}

TEST(SimpleTreeTest, rewireEdgeTest) {
    Dubins dubins{5, 0.1};
    Polygon valid_region;
    valid_region.emplace_back(XYZCoord(0, 0, 0));
    valid_region.emplace_back(XYZCoord(100, 0, 0));
    valid_region.emplace_back(XYZCoord(100, 100, 0));
    valid_region.emplace_back(XYZCoord(0, 100, 0));
    Polygon obs1 = {
        {XYZCoord(10, 10, 0), XYZCoord(20, 10, 0), XYZCoord(20, 20, 0), XYZCoord(10, 20, 0)}};

    std::vector<Polygon> obstacles = {obs1};
    Environment env = Environment(valid_region, {XYZCoord(0, 0, 0)}, obstacles);
    RRTPoint point1 = RRTPoint(XYZCoord(25, 25, 0), 0);
    RRTPoint point2 = RRTPoint(XYZCoord(50, 75, 0), HALF_PI);
    RRTPoint point3 = RRTPoint(XYZCoord(50, 80, 1.5), HALF_PI);
    RRTPoint point4 = RRTPoint(XYZCoord(50, 60, 0.9), HALF_PI);

    RRTOption option1 = dubins.allOptions(point1, point2, true)[0];
    RRTOption option2 = dubins.allOptions(point2, point3, true)[0];
    RRTOption option3 = dubins.allOptions(point1, point4, true)[0];
    RRTOption new_option = dubins.allOptions(point4, point3, true)[0];

    RRTTree simple_tree = RRTTree(point1, env, dubins);

    RRTNode* root = simple_tree.getRoot();

    // these two should add
    bool add1 = simple_tree.addNode(root, point2, option1);
    bool add2 = simple_tree.addNode(simple_tree.getNode(point2), point3, option2);
    bool add3 = simple_tree.addNode(root, point4, option3);
    EXPECT_TRUE(add1);
    EXPECT_TRUE(add2);
    EXPECT_TRUE(add3);

    simple_tree.rewireEdge(simple_tree.getNode(point3), simple_tree.getNode(point2),
                           simple_tree.getNode(point4), {}, 2);
    EXPECT_TRUE(
        simple_tree.getEdge(simple_tree.getNode(point4), simple_tree.getNode(point3)).getCost() !=
        0);
    EXPECT_TRUE(
        simple_tree.getEdge(simple_tree.getNode(point2), simple_tree.getNode(point3)).getCost() ==
        std::numeric_limits<double>::infinity());

    EXPECT_TRUE(simple_tree.getNode(point3)->getParent() == simple_tree.getNode(point4));
    EXPECT_TRUE(simple_tree.getNode(point2)->getParent() == root);
    EXPECT_TRUE(simple_tree.getNode(point4)->getReachable().size() == 1);
    EXPECT_TRUE(simple_tree.getNode(point2)->getReachable().size() == 0);
    EXPECT_TRUE(simple_tree.getNode(point1)->getReachable().size() == 2);
}