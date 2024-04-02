#include "pathing/environment.hpp"

#include <gtest/gtest.h>

#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"


/*
 *   tests Polygon::pointInBounds
 */
TEST(EnvironmentTest, PointInBounds) {
    Polygon test;
    test.emplace_back(XYZCoord{1, 1, 0});
    test.emplace_back(XYZCoord{0, 1, 0});
    test.emplace_back(XYZCoord{0, 0, 0});
    test.emplace_back(XYZCoord{1, 0, 0});
    Environment test_env = {test, {XYZCoord(0, 0, 0)}, {}};

    EXPECT_EQ(true, test_env.isPointInPolygon(test, XYZCoord{0.5, 0.5, 0}));
    EXPECT_EQ(true, test_env.isPointInPolygon(test, XYZCoord{0.5, 0.5, 99999999}));

    EXPECT_EQ(false, test_env.isPointInPolygon(test, XYZCoord{1, 0.5, 0}));   // edge is outside
    EXPECT_EQ(false, test_env.isPointInPolygon(test, XYZCoord{2, 0.5, 0}));   // right
    EXPECT_EQ(false, test_env.isPointInPolygon(test, XYZCoord{0.5, 2, 0}));   // top
    EXPECT_EQ(false, test_env.isPointInPolygon(test, XYZCoord{-1, 0.5, 0}));  // left
    EXPECT_EQ(false, test_env.isPointInPolygon(test, XYZCoord{0.5, -1, 0}));  // down

    Polygon no_point = {};
    Environment no_point_env = {no_point, {XYZCoord(0, 0, 0)}, {}};

    EXPECT_EQ(false, no_point_env.isPointInPolygon(no_point, XYZCoord{1, 1, 1}));
    EXPECT_EQ(false, no_point_env.isPointInPolygon(no_point, XYZCoord{1, 0, 1}));
    EXPECT_EQ(false, no_point_env.isPointInPolygon(no_point, XYZCoord{0, 1, 1}));

    Polygon point;
    point.emplace_back(XYZCoord{1, 1, 1});
    Environment point_env = {point, {XYZCoord(0, 0, 0)}, {}};

    EXPECT_EQ(false, point_env.isPointInPolygon(point, XYZCoord{1, 1, 1}));
    EXPECT_EQ(false, point_env.isPointInPolygon(point, XYZCoord{1, 0, 1}));
    EXPECT_EQ(false, point_env.isPointInPolygon(point, XYZCoord{0, 1, 1}));

    // tests close to diagonals
    Polygon quadrilateral;
    quadrilateral.emplace_back(XYZCoord{0, 0, 0});
    quadrilateral.emplace_back(XYZCoord{2, 1, 0});
    quadrilateral.emplace_back(XYZCoord{4, 4, 0});
    quadrilateral.emplace_back(XYZCoord{1, 2, 0});
    Environment quadrilateral_env = {quadrilateral, {XYZCoord(0, 0, 0)}, {}};

    EXPECT_EQ(true, quadrilateral_env.isPointInPolygon(quadrilateral, XYZCoord{1.5, 1.00, 0}));
    EXPECT_EQ(true, quadrilateral_env.isPointInPolygon(quadrilateral, XYZCoord{0.5, 0.90, 0}));
    EXPECT_EQ(true, quadrilateral_env.isPointInPolygon(quadrilateral, XYZCoord{2.5, 2.00, 0}));
    EXPECT_EQ(true, quadrilateral_env.isPointInPolygon(quadrilateral, XYZCoord{1.5, 2.25, 0}));

    EXPECT_EQ(false, quadrilateral_env.isPointInPolygon(quadrilateral, XYZCoord{1.5, 0.75, 0}));
    EXPECT_EQ(false, quadrilateral_env.isPointInPolygon(quadrilateral, XYZCoord{0.5, 1.10, 0}));
    EXPECT_EQ(false, quadrilateral_env.isPointInPolygon(quadrilateral, XYZCoord{2.5, 1.30, 0}));
    EXPECT_EQ(false, quadrilateral_env.isPointInPolygon(quadrilateral, XYZCoord{1.5, 2.50, 0}));
}


/*
 *  Tests Environment::isPointInBounds
 *
 *  Mimics the Polygon::pointInBounds test
 */
TEST(EnvironmentTest, PointOutOfBoundsTest) {
    // Create an instance of the Environment class
    // square basic region
    Polygon small_square;
    small_square.emplace_back(XYZCoord{1, 1, 0});
    small_square.emplace_back(XYZCoord{0, 1, 0});
    small_square.emplace_back(XYZCoord{0, 0, 0});
    small_square.emplace_back(XYZCoord{1, 0, 0});
    Polygon obs1 = {
        {XYZCoord(10, 10, 0), XYZCoord(20, 10, 0), XYZCoord(20, 20, 0), XYZCoord(10, 20, 0)}};

    std::vector<Polygon> obstacles = {obs1};
    Environment test{small_square, {XYZCoord(0, 0, 0)}, obstacles};

    EXPECT_EQ(true, test.isPointInBounds(XYZCoord{0.5, 0.5, 0}));
    EXPECT_EQ(true, test.isPointInBounds(XYZCoord{0.5, 0.5, 99999999}));

    EXPECT_EQ(false, test.isPointInBounds(XYZCoord{1, 0.5, 0}));   // edge is outside
    EXPECT_EQ(false, test.isPointInBounds(XYZCoord{2, 0.5, 0}));   // right
    EXPECT_EQ(false, test.isPointInBounds(XYZCoord{0.5, 2, 0}));   // top
    EXPECT_EQ(false, test.isPointInBounds(XYZCoord{-1, 0.5, 0}));  // left
    EXPECT_EQ(false, test.isPointInBounds(XYZCoord{0.5, -1, 0}));  // down

    Polygon no_point_polygon;
    Environment no_point = {no_point_polygon, {XYZCoord(0, 0, 0)}, obstacles};

    EXPECT_EQ(false, no_point.isPointInBounds(XYZCoord{1, 1, 1}));
    EXPECT_EQ(false, no_point.isPointInBounds(XYZCoord{1, 0, 1}));
    EXPECT_EQ(false, no_point.isPointInBounds(XYZCoord{0, 1, 1}));

    Polygon point_polygon;
    point_polygon.emplace_back(XYZCoord{1, 1, 1});

    Environment point = {point_polygon, {XYZCoord(0, 0, 0)}, obstacles};

    EXPECT_EQ(false, point.isPointInBounds(XYZCoord{1, 1, 1}));
    EXPECT_EQ(false, point.isPointInBounds(XYZCoord{1, 0, 1}));
    EXPECT_EQ(false, point.isPointInBounds(XYZCoord{0, 1, 1}));

    // tests close to diagonals
    Polygon quadrateral_polygon;
    quadrateral_polygon.emplace_back(XYZCoord{0, 0, 0});
    quadrateral_polygon.emplace_back(XYZCoord{2, 1, 0});
    quadrateral_polygon.emplace_back(XYZCoord{4, 4, 0});
    quadrateral_polygon.emplace_back(XYZCoord{1, 2, 0});

    Environment quadrilateral = {quadrateral_polygon, {XYZCoord(0, 0, 0)}, obstacles};

    EXPECT_EQ(true, quadrilateral.isPointInBounds(XYZCoord{1.5, 1.00, 0}));
    EXPECT_EQ(true, quadrilateral.isPointInBounds(XYZCoord{0.5, 0.90, 0}));
    EXPECT_EQ(true, quadrilateral.isPointInBounds(XYZCoord{2.5, 2.00, 0}));
    EXPECT_EQ(true, quadrilateral.isPointInBounds(XYZCoord{1.5, 2.25, 0}));

    EXPECT_EQ(false, quadrilateral.isPointInBounds(XYZCoord{1.5, 0.75, 0}));
    EXPECT_EQ(false, quadrilateral.isPointInBounds(XYZCoord{0.5, 1.10, 0}));
    EXPECT_EQ(false, quadrilateral.isPointInBounds(XYZCoord{2.5, 1.30, 0}));
    EXPECT_EQ(false, quadrilateral.isPointInBounds(XYZCoord{1.5, 2.50, 0}));
}

/*
 *  Tests Environment::isPathInBounds
 *
 *  TODO - use an actual gnerated dubins path
 */
TEST(EnvironmentTest, PathOutOfBoundsTest) {
    // Create an instance of the Environment class
    // square basic region
    Polygon small_square;
    small_square.emplace_back(XYZCoord{1, 1, 0});
    small_square.emplace_back(XYZCoord{0, 1, 0});
    small_square.emplace_back(XYZCoord{0, 0, 0});
    small_square.emplace_back(XYZCoord{1, 0, 0});

    Polygon obs1 = {
        {XYZCoord(10, 10, 0), XYZCoord(20, 10, 0), XYZCoord(20, 20, 0), XYZCoord(10, 20, 0)}};

    std::vector<Polygon> obstacles = {obs1};

    Environment test{small_square, {XYZCoord(0, 0, 0)}, obstacles};

    // TODO --> REALLY SHIT TEST
    std::vector<XYZCoord> path_in_bounds{XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999},
                                         XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999}};

    std::vector<XYZCoord> path_out_of_bounds{XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999},
                                             XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999},
                                             XYZCoord{1, 0.5, 0},  // edge is outside
                                             XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999},
                                             XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999}};

    EXPECT_EQ(true, test.isPathInBounds(path_in_bounds));
    EXPECT_EQ(false, test.isPathInBounds(path_out_of_bounds));
}

/*
*   test points in bound and in obstacles
*/
TEST(EnvironmentTest, InsideObstacleTest) {
    Polygon square;
    square.emplace_back(XYZCoord{0, 0, 0});
    square.emplace_back(XYZCoord{100, 0, 0});
    square.emplace_back(XYZCoord{100, 100, 0});
    square.emplace_back(XYZCoord{0, 100, 0});

    Polygon obs1 = {
        {XYZCoord(10, 10, 0), XYZCoord(20, 10, 0), XYZCoord(20, 20, 0), XYZCoord(10, 20, 0)}};

    std::vector<Polygon> obstacles = {obs1};

    Environment test{square, {XYZCoord(0, 0, 0)}, obstacles};
    
    EXPECT_FALSE(test.isPointInBounds(XYZCoord{15, 15, 0}));
    EXPECT_FALSE(test.isPointInBounds(XYZCoord{10, 10, 0}));
    EXPECT_FALSE(test.isPointInBounds(XYZCoord{15, 10, 0}));

    // for sanity
    EXPECT_TRUE(test.isPointInBounds(XYZCoord{1, 1, 0}));
}

/*
*   tests Environment::intersect()
*/
TEST(EnvironmentTest, IntersectTest) {
    Environment test({}, {}, {});

    // test intersect
    std::vector<XYZCoord> path1 = {XYZCoord{0, 0, 0}, XYZCoord{100, 100, 0}};
    std::vector<XYZCoord> path1_5 = {XYZCoord{0, 0, 0}, XYZCoord{-1, -1, 0}}; 
    std::vector<XYZCoord> path2 = {XYZCoord{-1, -1, 0}, XYZCoord{5, 5, 0}};
    std::vector<XYZCoord> path3 = {XYZCoord{0, 0, 0}, XYZCoord{15, 15, 0}};
    std::vector<XYZCoord> path4 = {XYZCoord{2, 1, 0}, XYZCoord{1, 2, 0}};

    EXPECT_TRUE(test.intersect(path1[0], path1[1], path2[0], path2[1]));
    EXPECT_TRUE(test.intersect(path1[0], path1[1], path1_5[0], path1_5[1]));

    EXPECT_TRUE(test.intersect(path1[0], path1[1], path3[0], path3[1]));
    EXPECT_TRUE(test.intersect(path2[0], path2[1], path3[0], path3[1]));

    EXPECT_TRUE(test.intersect(path1[0], path1[1], path4[0], path4[1]));
    EXPECT_FALSE(test.intersect(path1_5[0], path1_5[1], path4[0], path4[1]));


}