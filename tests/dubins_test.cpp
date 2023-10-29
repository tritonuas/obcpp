#include <gtest/gtest.h>
#include "../include/pathing/dubins.hpp"
#include "Eigen"
/*
*   
*
*
*/
TEST(DubinsTest, Orthogonal2D) {
    Eigen::Vector2d input_vector1(1.0, 2.0);
    Eigen::Vector2d input_vector2(-3.0, 4.0);
    Eigen::Vector2d input_vector3(0.0, 0.0);

    Eigen::Vector2d expected_output1(-2.0, 1.0);
    Eigen::Vector2d expected_output2(-4.0, -3.0);
    Eigen::Vector2d expected_output3(0.0, 0.0);

    EXPECT_EQ(findOrthogonalVector2D(input_vector1), expected_output1);
}

