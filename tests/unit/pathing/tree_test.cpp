#include "pathing/tree.hpp"
#include <iostream> 
#include <gtest/gtest.h>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/common.hpp"

/*
 *   very bad tests, was too lazy to check if every parameter was correct, aka didn't bother to find
 *   the hardcoded values for the expected values.
 */
#include <iostream>

TEST(CheckCounterClockwiseTest, CounterClockwiseTest){
    // testing if flightbounds counter clockwise returns correctly 
    GPSCoord GPS4 = makeGPSCoord(0,0,0);
    GPSCoord GPS3 = makeGPSCoord(10,0,0);
    GPSCoord GPS2 = makeGPSCoord(10,10,0);
    GPSCoord GPS1 = makeGPSCoord(0,10,0);

    std::vector<GPSCoord> coords_Test1 = {GPS1, GPS2, GPS3, GPS4}; 

    int expected_val1 = checkCounterClockwise(coords_Test1);
    
    EXPECT_TRUE(expected_val1 == 0);
    //test with actual coords
    GPSCoord GEISEL = makeGPSCoord(32.88146, -117.23772,0);
    std::cout << "If this appears coordinate1 was created propely" << std::endl; 
    GPSCoord PINES = makeGPSCoord(32.87886,-117.24249,0);
    std::cout << "If this appears coordinate2 was created propely" << std::endl; 
    GPSCoord FAH = makeGPSCoord(32.88354,-117.23496,0);
    std::cout << "If this appears coordinate3 was created propely" << std::endl;
    GPSCoord SUNGOD = makeGPSCoord(32.87864,-117.23977,0);
    std::cout << "If this appears coordinate4 was created propely" << std::endl;

    std::vector<GPSCoord> coords_Test2 = {GEISEL, PINES, SUNGOD, FAH}; 

    std::cout << "If this appears coord list was created properly" << std::endl;

    int expected_val2 = checkCounterClockwise(coords_Test2);
    std::cout << "If this appears Triple C ran" << std::endl;
    
    EXPECT_TRUE(expected_val2 == 0);

    std::vector<GPSCoord> coords_Test3 = {GEISEL, SUNGOD, PINES, FAH};

    int expected_val3 = checkCounterClockwise(coords_Test3);
    
    EXPECT_TRUE(expected_val3 == 1);
}

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
    Environment env = Environment(valid_region, {}, {}, {XYZCoord(0, 0, 0)}, obstacles);
    RRTPoint point1 = RRTPoint(XYZCoord(25, 25, 0), 0);
    RRTPoint point2 = RRTPoint(XYZCoord(50, 75, 0), 0);
    RRTOption option = dubins.allOptions(point1, point2, true)[0];

    RRTTree simple_tree = RRTTree(point1, env, dubins);

    std::shared_ptr<RRTNode> root = simple_tree.getRoot();

    // simpleTree.addNode(root, point1);
    std::shared_ptr<RRTNode> added_point = simple_tree.addSample(root, point2, option);

    EXPECT_TRUE(added_point != nullptr);
    EXPECT_TRUE(root->getReachable().size() == 1);
    EXPECT_TRUE(root->getReachable()[0]->getPoint() == point2);
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
    Environment env = Environment(valid_region, {}, {}, {XYZCoord(0, 0, 0)}, obstacles);
    RRTPoint point1 = RRTPoint(XYZCoord(25, 25, 0), 0);
    RRTPoint point2 = RRTPoint(XYZCoord(50, 75, 0), HALF_PI);
    RRTPoint point3 = RRTPoint(XYZCoord(50, 80, 1.5), HALF_PI);
    RRTPoint point4 = RRTPoint(XYZCoord(50, 60, 0.9), HALF_PI);

    RRTOption option1 = dubins.allOptions(point1, point2, true)[0];
    RRTOption option2 = dubins.allOptions(point2, point3, true)[0];
    RRTOption option3 = dubins.allOptions(point1, point4, true)[0];
    RRTOption new_option = dubins.allOptions(point4, point3, true)[0];

    RRTTree simple_tree = RRTTree(point1, env, dubins);

    std::shared_ptr<RRTNode> root = simple_tree.getRoot();

    // these two should add
    std::shared_ptr<RRTNode> node2 = simple_tree.addSample(root, point2, option1);
    std::shared_ptr<RRTNode> node3 = simple_tree.addSample(node2, point3, option2);
    std::shared_ptr<RRTNode> node4 = simple_tree.addSample(root, point4, option3);
    EXPECT_TRUE(node2 != nullptr);
    EXPECT_TRUE(node3 != nullptr);
    EXPECT_TRUE(node4 != nullptr);

    simple_tree.rewireEdge(node3, node2, node4, {}, 2);

    EXPECT_TRUE(node3->getPathLength() != 0);

    // EXPECT_TRUE(simple_tree.getEdge(node2, node3).getCost() ==
    // std::numeric_limits<double>::infinity());

    EXPECT_TRUE(node4->getReachable().size() == 1);
    EXPECT_TRUE(node2->getReachable().size() == 0);
    EXPECT_TRUE(root->getReachable().size() == 2);
}