#include <gtest/gtest.h>
#include <math.h>

#include "../include/pathing/dubins.hpp"
#include "../include/utilities/datatypes.hpp"

#include "Eigen"
/*
 *   [TODO]
 *   - separate failed tests from success tests (if there are any)
 */

/*
 *   Tests dubins ==> findOrthogonalVector2D()
 */
TEST(DubinsTest, Orthogonal2D)
{
    Eigen::Vector2d input_vector1(1.0, 2.0);
    Eigen::Vector2d input_vector2(-3.0, 4.0);
    Eigen::Vector2d input_vector3(0.0, 0.0); // origin
    Eigen::Vector2d input_vector4(1.0, 0.0); // e1 basis vector
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
TEST(DubinsTest, DistanceBetweenVectors)
{
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
TEST(DubinsTest, Midpoint)
{
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

    EXPECT_NEAR(result_vector4[0], midpoint4[0], 0.01);
    EXPECT_NEAR(result_vector4[1], midpoint4[1], 0.01);
    EXPECT_NEAR(result_vector5[0], midpoint5[0], 0.01);
    EXPECT_NEAR(result_vector5[1], midpoint5[1], 0.01);
}

/*
 *   tests Dubins::findCenter()
 */
TEST(DubinsTest, FindCenter)
{
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
 *   tests Dubins::circleArc()
 *   (poorly)
 *
 *   [TODO] - add more tests (that are not trivial)
 */
TEST(DubinsTest, CircleArc)
{
    Dubins dubins1(5, 10);

    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord origin_y(0, 0, 0, M_PI / 2);
    XYZCoord arbitrary_position(73, 41, 0, 4.00);

    // plane is facing x+, turning left/ccw with a turning radius of 5,
    // this should be the point where it turns 90deg (1/4 of the circle)
    Eigen::Vector2d result1 = dubins1.circleArc(origin_x, 1,
                                                dubins1.findCenter(origin_x, 'L'), M_PI / 2 * 5);
    Eigen::Vector2d expected_result1(5.0, 5.0);

    // plance facing x+, turning right/cw with a turning radius of 5
    // turning 2.97 rad
    Eigen::Vector2d result2 = dubins1.circleArc(origin_x, -1,
                                                dubins1.findCenter(origin_x, 'R'), 2.97 * 5);
    Eigen::Vector2d expected_result2(0.850, -9.927);

    Eigen::Vector2d result3 = dubins1.circleArc(arbitrary_position, 1,
                                                dubins1.findCenter(arbitrary_position, 'L'), 5.12 * 5);
    Eigen::Vector2d expected_result3(78.28441936, 42.50134993);

    Eigen::Vector2d result4 = dubins1.circleArc(origin_y, 1,
                                                dubins1.findCenter(origin_y, 'L'), M_PI * 5);
    Eigen::Vector2d expected_result4(-10.0, 0.0);

    EXPECT_NEAR(result1[0], expected_result1[0], 0.01);
    EXPECT_NEAR(result1[1], expected_result1[1], 0.01);

    EXPECT_NEAR(result2[0], expected_result2[0], 0.01);
    EXPECT_NEAR(result2[1], expected_result2[1], 0.01);

    EXPECT_NEAR(result3[0], expected_result3[0], 0.01);
    EXPECT_NEAR(result3[1], expected_result3[1], 0.01);

    EXPECT_NEAR(result4[0], expected_result4[0], 0.01);
    EXPECT_NEAR(result4[1], expected_result4[1], 0.01);
}

/*
 *   tests Dubins::generatePointsStraight()
 */
TEST(DubinsTest, GenPointsStraight)
{
    Dubins dubins1(5, 10);

    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord origin_y(0, 0, 0, M_PI / 2);
    XYZCoord arbitrary_position(73, 41, 0, 4.00);
}
/*
 *   tests Dubins::generatePointsCurve()
 */
TEST(DubinsTest, GenPointsCurve)
{
    EXPECT_EQ(5, 5);
}

/*
 *   tests Dubins::lsl()
 *   [TODO] - make more tests (including trivial tests)
 */
TEST(DubinsTest, LSL)
{
    Dubins dubins1(5, 10);

    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord origin_y(0, 0, 0, M_PI / 2);
    XYZCoord arbitrary_position1(73, 41, 0, 4.00);
    XYZCoord plus_x100(100, 0, 0, 0);
    XYZCoord arbitrary_position2(5, 100, 0, M_PI / 2);

    RRTOption result1 = dubins1.lsl(origin_x, arbitrary_position1,
                                    dubins1.findCenter(origin_x, 'L'), dubins1.findCenter(arbitrary_position1, 'L'));
    RRTOption expected_result1(103.46948015930067, DubinsPath(0.40295754, 3.5970424510, 83.46948015930067), true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = dubins1.lsl(origin_x, plus_x100,
                                    dubins1.findCenter(origin_x, 'L'), dubins1.findCenter(plus_x100, 'L'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 = dubins1.lsl(origin_x, arbitrary_position2,
                                    dubins1.findCenter(origin_x, 'L'), dubins1.findCenter(arbitrary_position2, 'L'));
    RRTOption expected_result3(102.85398163397448, DubinsPath(M_PI / 2, 0, 95), true);

    EXPECT_NEAR(result3.length, expected_result3.length, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_0, expected_result3.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_2, expected_result3.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result3.length, expected_result3.length, 0.01);
    EXPECT_EQ(result3.has_straight, expected_result3.has_straight);
}

/*
 *   tests Dubins::rsr()
 */
TEST(DubinsTest, RSR)
{
    Dubins dubins1(5, 10);
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord origin_y(0, 0, 0, M_PI / 2);
    XYZCoord arbitrary_position1(73, 41, 0, 4.00);
    XYZCoord plus_x100(100, 0, 0, 0);
    XYZCoord arbitrary_position2(5, -100, 0, 3 * M_PI / 2);

    RRTOption result1 = dubins1.rsr(origin_x, arbitrary_position1,
                                    dubins1.findCenter(origin_x, 'R'), dubins1.findCenter(arbitrary_position1, 'R'));
    RRTOption expected_result1(127.792, DubinsPath(-5.664581035483313, -2.9017895788758596, 84.96005087111514), true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = dubins1.rsr(origin_x, plus_x100,
                                    dubins1.findCenter(origin_x, 'R'), dubins1.findCenter(plus_x100, 'R'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 = dubins1.rsr(origin_x, arbitrary_position2,
                                    dubins1.findCenter(origin_x, 'R'), dubins1.findCenter(arbitrary_position2, 'R'));
    RRTOption expected_result3(102.85398163397448, DubinsPath(-M_PI / 2, 0, 95), true);

    EXPECT_NEAR(result3.length, expected_result3.length, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_0, expected_result3.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_2, expected_result3.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result3.length, expected_result3.length, 0.01);
    EXPECT_EQ(result3.has_straight, expected_result3.has_straight);
}

/*
 *   tests Dubins::rsl()
 */
TEST(DubinsTest, RSL)
{
    EXPECT_EQ(5, 5);
}

/*
 *   tests Dubins::lsr()
 */
TEST(DubinsTest, LSR)
{
    EXPECT_EQ(5, 5);
}

/*
 *   tests Dubins::lrl()
 */
TEST(DubinsTest, LRL)
{
    EXPECT_EQ(5, 5);
}

/*
 *   tests Dubins::rlr()
 */
TEST(DubinsTest, RLR)
{
    EXPECT_EQ(5, 5);
}

/*
 *   tests Dubins::allOptions()
 */
TEST(DubinsTest, AllOptions)
{
    EXPECT_EQ(5, 5);
}

/*
 *   tests Dubins::dubinsPath()
 */
TEST(DubinsTest, DubinsPath)
{
    EXPECT_EQ(5, 5);
}