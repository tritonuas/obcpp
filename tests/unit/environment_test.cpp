#include "pathing/environment.hpp"

#include <gtest/gtest.h>

#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

/*
 *  Tests Environment::isPointInBounds
 *
 *  Mimics the Polygon::pointInBounds test
 */
TEST(EnvironmentTest, PointOutOfBoundsTest) {
    // Create an instance of the Environment class
    // square basic region
    Polygon small_square{FLIGHT_BOUND_COLOR};
    small_square.emplace_back(XYZCoord{1, 1, 0});
    small_square.emplace_back(XYZCoord{0, 1, 0});
    small_square.emplace_back(XYZCoord{0, 0, 0});
    small_square.emplace_back(XYZCoord{1, 0, 0});

    Environment test{small_square, RRTPoint(XYZCoord(0, 0, 0), 0), 0};

    EXPECT_EQ(true, test.isPointInBounds(XYZCoord{0.5, 0.5, 0}));
    EXPECT_EQ(true, test.isPointInBounds(XYZCoord{0.5, 0.5, 99999999}));

    EXPECT_EQ(false, test.isPointInBounds(XYZCoord{1, 0.5, 0}));   // edge is outside
    EXPECT_EQ(false, test.isPointInBounds(XYZCoord{2, 0.5, 0}));   // right
    EXPECT_EQ(false, test.isPointInBounds(XYZCoord{0.5, 2, 0}));   // top
    EXPECT_EQ(false, test.isPointInBounds(XYZCoord{-1, 0.5, 0}));  // left
    EXPECT_EQ(false, test.isPointInBounds(XYZCoord{0.5, -1, 0}));  // down

    Polygon no_point_polygon{FLIGHT_BOUND_COLOR};
    Environment no_point = {no_point_polygon, RRTPoint(XYZCoord(0, 0, 0), 0), 0};

    EXPECT_EQ(false, no_point.isPointInBounds(XYZCoord{1, 1, 1}));
    EXPECT_EQ(false, no_point.isPointInBounds(XYZCoord{1, 0, 1}));
    EXPECT_EQ(false, no_point.isPointInBounds(XYZCoord{0, 1, 1}));

    Polygon point_polygon{FLIGHT_BOUND_COLOR};
    point_polygon.emplace_back(XYZCoord{1, 1, 1});

    Environment point = {point_polygon, RRTPoint(XYZCoord(0, 0, 0), 0), 0};

    EXPECT_EQ(false, point.isPointInBounds(XYZCoord{1, 1, 1}));
    EXPECT_EQ(false, point.isPointInBounds(XYZCoord{1, 0, 1}));
    EXPECT_EQ(false, point.isPointInBounds(XYZCoord{0, 1, 1}));

    // tests close to diagonals
    Polygon quadrateral_polygon{FLIGHT_BOUND_COLOR};
    quadrateral_polygon.emplace_back(XYZCoord{0, 0, 0});
    quadrateral_polygon.emplace_back(XYZCoord{2, 1, 0});
    quadrateral_polygon.emplace_back(XYZCoord{4, 4, 0});
    quadrateral_polygon.emplace_back(XYZCoord{1, 2, 0});

    Environment quadrilateral = {quadrateral_polygon, RRTPoint(XYZCoord(0, 0, 0), 0), 0};

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
    Polygon small_square{FLIGHT_BOUND_COLOR};
    small_square.emplace_back(XYZCoord{1, 1, 0});
    small_square.emplace_back(XYZCoord{0, 1, 0});
    small_square.emplace_back(XYZCoord{0, 0, 0});
    small_square.emplace_back(XYZCoord{1, 0, 0});

    Environment test{small_square, RRTPoint(XYZCoord(0, 0, 0), 0), 0};

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


TEST(EnvironmentTest, IsPointInGoalTest) {
    Polygon small_square{FLIGHT_BOUND_COLOR};
    small_square.emplace_back(XYZCoord{1, 1, 0});
    small_square.emplace_back(XYZCoord{0, 1, 0});
    small_square.emplace_back(XYZCoord{0, 0, 0});
    small_square.emplace_back(XYZCoord{1, 0, 0});

    Environment test{small_square, RRTPoint(XYZCoord(0, 0, 0), 0), 5};

    EXPECT_EQ(true, test.isPointInGoal(XYZCoord{0, 0, 0}));
    EXPECT_EQ(true, test.isPointInGoal(XYZCoord{1, 1, 0}));
    EXPECT_EQ(true, test.isPointInGoal(XYZCoord{1, 1, 100000000}));
    EXPECT_EQ(true, test.isPointInGoal(XYZCoord{5, 0, 0}));
    EXPECT_EQ(true, test.isPointInGoal(XYZCoord{2, 1, 0}));
    EXPECT_EQ(false, test.isPointInGoal(XYZCoord{5.00001, 0, 0})); 
}
