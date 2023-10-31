#include <gtest/gtest.h>
#include <math.h>

#include "../include/pathing/dubins.hpp"
#include "../include/utilities/datatypes.hpp"

#include "Eigen"

/*
*   Tests dubins ==> findOrthogonalVector2D()
*/
TEST(DubinsTest, Orthogonal2D) {
    Eigen::Vector2d input_vector1(1.0, 2.0);
    Eigen::Vector2d input_vector2(-3.0, 4.0);
    Eigen::Vector2d input_vector3(0.0, 0.0);    // origin
    Eigen::Vector2d input_vector4(1.0, 0.0);    // e1 basis vector
    Eigen::Vector2d input_vector5(-1.0, -1.0);

    Eigen::Vector2d expected_output1(-2.0, 1.0);
    Eigen::Vector2d expected_output2(-4.0, -3.0);
    Eigen::Vector2d expected_output3(0.0, 0.0); // should not change origin
    Eigen::Vector2d expected_output4(0.0, 1.0); // e2 basis vector (90 CCW rotation)
    Eigen::Vector2d expected_output5(1.0, -1.0);

    EXPECT_EQ(findOrthogonalVector2D(input_vector1), expected_output1);
    EXPECT_EQ(findOrthogonalVector2D(input_vector2), expected_output2);
    EXPECT_EQ(findOrthogonalVector2D(input_vector3), expected_output3);
    EXPECT_EQ(findOrthogonalVector2D(input_vector4), expected_output4);
    EXPECT_EQ(findOrthogonalVector2D(input_vector5), expected_output5);
}

/*
*   Tests dubins ==> distanceBetween()
*/
TEST(DubinsTest, DistanceBetweenVectors) {
    // 3-4-5 right triangle
    Eigen::Vector2d start_vector1(3.0, 0.0);
    Eigen::Vector2d end_vector1(0.0, 4.0);

    // trivial case
    Eigen::Vector2d start_vector2(0.0, 0.0);
    Eigen::Vector2d end_vector2(0.0, 0.0);

    // scalar multiples
    Eigen::Vector2d start_vector3(1.0, 0.0);
    Eigen::Vector2d end_vector3(2.0, 0.0);

    Eigen::Vector2d start_vector4(3.0, 4.0);
    Eigen::Vector2d end_vector4(0.0, 0.0);

    Eigen::Vector2d start_vector5(102.5, -125.5);
    Eigen::Vector2d end_vector5(1825.0, 2389.8);

    EXPECT_DOUBLE_EQ(distanceBetween(start_vector1, end_vector1), 5.0);
    EXPECT_DOUBLE_EQ(distanceBetween(start_vector2, end_vector2), 0.0);
    EXPECT_DOUBLE_EQ(distanceBetween(start_vector3, end_vector3), 1.0);
    EXPECT_DOUBLE_EQ(distanceBetween(start_vector4, end_vector4), 5.0);
    EXPECT_NEAR(distanceBetween(start_vector5, end_vector5), 3048.56, 0.1);
}

/*
*   tests dubins ==> midpoint()
*/
TEST(DubinsTest, Midpoint) {
    // 1] Results with Integer components
    Eigen::Vector2d start_vector1(0.0, 0.0);
    Eigen::Vector2d end_vector1(0.0, 0.0);
    Eigen::Vector2d midpoint1(0.0, 0.0);

    Eigen::Vector2d start_vector2(2.0, 0.0);
    Eigen::Vector2d end_vector2(0.0, 2.0);
    Eigen::Vector2d midpoint2(1.0, 1.0);

    Eigen::Vector2d start_vector3(2.0, 0.0);
    Eigen::Vector2d end_vector3(6.0, 0.0);
    Eigen::Vector2d midpoint3(4.0, 0.0);

    EXPECT_EQ(midpoint(start_vector1, end_vector1), midpoint1);
    EXPECT_EQ(midpoint(start_vector2, end_vector2), midpoint2);
    EXPECT_EQ(midpoint(start_vector3, end_vector3), midpoint3);

    // 2] Results without Integer components
    Eigen::Vector2d start_vector4(1.0, 0.0);
    Eigen::Vector2d end_vector4(0.0, 1.0);
    Eigen::Vector2d midpoint4(0.5, 0.5);

    Eigen::Vector2d start_vector5(102.5, -125.5);
    Eigen::Vector2d end_vector5(1825.0, 2389.8);
    Eigen::Vector2d midpoint5(963.75, 1132.15);

    Eigen::Vector2d result_vector4 = midpoint(start_vector4, end_vector4);
    Eigen::Vector2d result_vector5 = midpoint(start_vector5, end_vector5);

    EXPECT_NEAR(result_vector4[0], midpoint4[0], 0.1);
    EXPECT_NEAR(result_vector4[1], midpoint4[1], 0.1);
    EXPECT_NEAR(result_vector5[0], midpoint5[0], 0.1);
    EXPECT_NEAR(result_vector5[1], midpoint5[1], 0.1);
}

/*
*   tests Dubins::findCenter()
*/
TEST(DubinsTest, FindCenter) {
    Dubins dubins1(5, 10);
    
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    Eigen::Vector2d expected_result1(0.0, 5.0);
    Eigen::Vector2d expected_result2(0.0, -5.0);

    // points towards e2
    XYZCoord origin_y(0, 0, 0, M_PI / 2);
    Eigen::Vector2d expected_result3(-5.0, 0.0);
    Eigen::Vector2d expected_result4(5.0, 0.0);

    XYZCoord arbitrary(12, 156, 100, 1.3);
    // [-4.817, 1.341] ==> magnitude 5 * e1 vector rotated 2.87 [1.3 + pi/2] raidans
    Eigen::Vector2d expected_result5(12 - 4.817, 156 + 1.341);
    Eigen::Vector2d expected_result6(12 + 4.817, 156 - 1.341);

    // not precise enough to use EXPECT_EQ
    Eigen::Vector2d result1 = dubins1.findCenter(origin_x, 'L');
    Eigen::Vector2d result2 = dubins1.findCenter(origin_x, 'R');
    Eigen::Vector2d result3 = dubins1.findCenter(origin_y, 'L');
    Eigen::Vector2d result4 = dubins1.findCenter(origin_y, 'R');
    Eigen::Vector2d result5 = dubins1.findCenter(arbitrary, 'L');
    Eigen::Vector2d result6 = dubins1.findCenter(arbitrary, 'R');

    EXPECT_NEAR(result1[0], expected_result1[0], 0.01);
    EXPECT_NEAR(result1[1], expected_result1[1], 0.01);

    EXPECT_NEAR(result2[0], expected_result2[0], 0.01);
    EXPECT_NEAR(result2[1], expected_result2[1], 0.01);

    EXPECT_NEAR(result3[0], expected_result3[0], 0.01);
    EXPECT_NEAR(result3[1], expected_result3[1], 0.01);

    EXPECT_NEAR(result4[0], expected_result4[0], 0.01);
    EXPECT_NEAR(result4[1], expected_result4[1], 0.01);

    EXPECT_NEAR(result5[0], expected_result5[0], 0.01);
    EXPECT_NEAR(result5[1], expected_result5[1], 0.01);

    EXPECT_NEAR(result6[0], expected_result6[0], 0.01);
    EXPECT_NEAR(result6[1], expected_result6[1], 0.01);
}

/*
*   tests Dubins::circle_arc()
*   (poorly)
*
*   [TODO] - add more tests (that are not trivial)
*/
TEST(Dubinstest, CircleArc) {
    Dubins dubins1(5, 10);
    
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);

    // plane is facing x+, turning left with a turning radius of 5, 
    // this should be the point wheree it turns 90deg (1/4 of the circle)
    Eigen::Vector2d result1 = dubins1.circle_arc(origin_x, 1, 
                            dubins1.findCenter(origin_x, 'L'), M_PI / 2 * 5);
    Eigen::Vector2d expected_result1(5.0, 5.0);

    EXPECT_EQ(result1, expected_result1);
}