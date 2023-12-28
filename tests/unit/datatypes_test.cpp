#include <gtest/gtest.h>
#include <cmath>

#include "utilities/datatypes.hpp"

TEST(Datatypes, equals)
{
    XYZCoord origin{0, 0, 0};
    EXPECT_EQ(origin == (XYZCoord{0, 0, 0}), true);
    EXPECT_EQ(origin == (XYZCoord{1, 0, 0}), false);
    EXPECT_EQ(origin == (XYZCoord{0, 1, 0}), false);
    EXPECT_EQ(origin == (XYZCoord{0, 0, 1}), false);
    EXPECT_EQ(origin == (XYZCoord{1, 1, 0}), false);

    // make sure there is no double weirdness
    XYZCoord rand_point{0.1, -0.1, 1683};
    EXPECT_EQ(rand_point == (XYZCoord{0.1, -0.1, 1683}), true);
    EXPECT_EQ(rand_point == (XYZCoord{0, -0.1, 1683}), false);
    EXPECT_EQ(rand_point == (XYZCoord{0.1, 0.1, 1683}), false);
    EXPECT_EQ(rand_point == (XYZCoord{0.1, -0.1, 683}), false);

    EXPECT_EQ(rand_point == origin, false);
}

TEST(Datatypes, plusEquals)
{
    XYZCoord mutated_coord{0, 0, 0};
    EXPECT_EQ(mutated_coord, (XYZCoord{0, 0, 0}));

    mutated_coord += XYZCoord{0, 0, 0};
    EXPECT_EQ(mutated_coord, (XYZCoord{0, 0, 0}));

    mutated_coord += XYZCoord{1, 1, 1};
    EXPECT_EQ(mutated_coord, (XYZCoord{1, 1, 1}));

    mutated_coord += XYZCoord{15.263, -548526.3, 789.63};
    EXPECT_NEAR(mutated_coord.x, 16.263, 0.001);
    EXPECT_NEAR(mutated_coord.y, -548525.3, 0.001);
    EXPECT_NEAR(mutated_coord.z, 790.63, 0.001);
}

TEST(Datatypes, addition)
{
    XYZCoord origin{0, 0, 0};
    EXPECT_EQ(origin, (XYZCoord{0, 0, 0}));

    EXPECT_EQ(origin + (XYZCoord{0, 0, 0}), (XYZCoord{0, 0, 0}));

    EXPECT_EQ(origin + (XYZCoord{1, 1, 1}), (XYZCoord{1, 1, 1}));

    XYZCoord arbitrary_point =  XYZCoord{1, 1, 1} + XYZCoord{15.263, -548526.3, 789.63};
    EXPECT_NEAR(arbitrary_point.x, 16.263, 0.001);
    EXPECT_NEAR(arbitrary_point.y, -548525.3, 0.001);
    EXPECT_NEAR(arbitrary_point.z, 790.63, 0.001);
}


TEST(Datatypes, minusEquals)
{
    XYZCoord mutated_coord{0, 0, 0};
    EXPECT_EQ(mutated_coord, (XYZCoord{0, 0, 0}));

    mutated_coord -= XYZCoord{0, 0, 0};
    EXPECT_EQ(mutated_coord, (XYZCoord{0, 0, 0}));

    mutated_coord -= XYZCoord{1, 1, 1};
    EXPECT_EQ(mutated_coord, (XYZCoord{-1, -1, -1}));

    mutated_coord -= XYZCoord{15.263, -548526.3, 789.63};
    EXPECT_NEAR(mutated_coord.x, -16.263, 0.001);
    EXPECT_NEAR(mutated_coord.y, 548525.3, 0.001);
    EXPECT_NEAR(mutated_coord.z, -790.63, 0.001);
}

TEST(Datatypes, subtraction)
{
    XYZCoord origin{0, 0, 0};
    EXPECT_EQ(origin, (XYZCoord{0, 0, 0}));

    EXPECT_EQ(origin - (XYZCoord{0, 0, 0}), (XYZCoord{0, 0, 0}));

    EXPECT_EQ(origin - (XYZCoord{1, 1, 1}), (XYZCoord{-1, -1, -1}));

    XYZCoord arbitrary_point =  XYZCoord{-1, -1, -1} - XYZCoord{15.263, -548526.3, 789.63};
    EXPECT_NEAR(arbitrary_point.x, -16.263, 0.001);
    EXPECT_NEAR(arbitrary_point.y, 548525.3, 0.001);
    EXPECT_NEAR(arbitrary_point.z, -790.63, 0.001);
}

TEST(Datatypes, scalarMultiplication)
{
    XYZCoord ones{1,1,1};
    
    EXPECT_EQ(ones * 1, (XYZCoord{1,1,1}));
    EXPECT_EQ(1 * ones, (XYZCoord{1,1,1}));
    EXPECT_EQ(1 * ones, ones * 1);

    EXPECT_EQ(ones * 0, (XYZCoord{0,0,0}));
    EXPECT_EQ(0 * ones, (XYZCoord{0,0,0}));
    EXPECT_EQ(0 * ones, ones * 0);

    EXPECT_EQ(ones * 8, (XYZCoord{8,8,8}));
    EXPECT_EQ(8 * ones, (XYZCoord{8,8,8}));
    EXPECT_EQ(8 * ones, ones * 8);

    EXPECT_EQ(ones * -2.61, (XYZCoord{-2.61,-2.61,-2.61}));
    EXPECT_EQ(-2.61 * ones, (XYZCoord{-2.61,-2.61,-2.61}));
    EXPECT_EQ(-2.61 * ones, ones * -2.61);

    XYZCoord double_vector = XYZCoord{5.26, 48.54, -4.523};
    XYZCoord multiply_from_left = 3.526 * double_vector;
    XYZCoord multiply_from_right = double_vector * 3.526;

    EXPECT_EQ(multiply_from_left, multiply_from_right);

    EXPECT_NEAR(multiply_from_left.x, 18.54676, 0.001);
    EXPECT_NEAR(multiply_from_left.y, 171.15204, 0.001);
    EXPECT_NEAR(multiply_from_left.z, -15.948098, 0.001);

    EXPECT_NEAR(multiply_from_right.x, 18.54676, 0.001);
    EXPECT_NEAR(multiply_from_right.y, 171.15204, 0.001);
    EXPECT_NEAR(multiply_from_right.z, -15.948098, 0.001);
}