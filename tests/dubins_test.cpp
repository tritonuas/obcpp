#include <gtest/gtest.h>
#include <math.h>

#include "../include/pathing/dubins.hpp"
#include "../include/utilities/datatypes.hpp"

#include "Eigen"
/*
 *   [TODO]
 *      - delete unnessisary var
 *      - standardize namingconventions, and test order
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
 *   tests dubins ==> halfDisplacement()
 */
TEST(DubinsTest, HalfDisplacement)
{
    // 1] Results with Integer components
    Eigen::Vector2d start_vector1(0.0, 0.0);
    Eigen::Vector2d end_vector1(0.0, 0.0);
    Eigen::Vector2d half_displacement1(0.0, 0.0);

    Eigen::Vector2d start_vector2(2.0, 0.0);
    Eigen::Vector2d end_vector2(0.0, 2.0);
    Eigen::Vector2d half_displacement2(1.0, -1.0);

    Eigen::Vector2d start_vector3(2.0, 0.0);
    Eigen::Vector2d end_vector3(6.0, 0.0);
    Eigen::Vector2d half_displacement3(-2.0, 0.0);

    EXPECT_EQ(halfDisplacement(start_vector1, end_vector1), half_displacement1);
    EXPECT_EQ(halfDisplacement(start_vector2, end_vector2), half_displacement2);
    EXPECT_EQ(halfDisplacement(start_vector3, end_vector3), half_displacement3);

    // 2] Results without Integer components
    Eigen::Vector2d start_vector4(1.0, 0.0);
    Eigen::Vector2d end_vector4(0.0, 1.0);
    Eigen::Vector2d half_displacement4(0.5, -0.5);

    Eigen::Vector2d start_vector5(102.5, -125.5);
    Eigen::Vector2d end_vector5(1825.0, 2389.8);
    Eigen::Vector2d half_displacement5(-861.25, -1257.65);

    Eigen::Vector2d result_vector4 = halfDisplacement(start_vector4, end_vector4);
    Eigen::Vector2d result_vector5 = halfDisplacement(start_vector5, end_vector5);

    EXPECT_NEAR(result_vector4[0], half_displacement4[0], 0.01);
    EXPECT_NEAR(result_vector4[1], half_displacement4[1], 0.01);
    EXPECT_NEAR(result_vector5[0], half_displacement5[0], 0.01);
    EXPECT_NEAR(result_vector5[1], half_displacement5[1], 0.01);
}

/*
 *   tests Dubins::findCenter()
 */
TEST(DubinsTest, FindCenter)
{
    Dubins dubins1(5, 10);

    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    Eigen::Vector2d result1 = dubins1.findCenter(origin_x, 'L');
    Eigen::Vector2d expected_result1(0.0, 5.0);
    Eigen::Vector2d result2 = dubins1.findCenter(origin_x, 'R');
    Eigen::Vector2d expected_result2(0.0, -5.0);

    // points towards e2
    XYZCoord origin_y(0, 0, 0, M_PI / 2);
    Eigen::Vector2d result3 = dubins1.findCenter(origin_y, 'L');
    Eigen::Vector2d expected_result3(-5.0, 0.0);
    Eigen::Vector2d result4 = dubins1.findCenter(origin_y, 'R');
    Eigen::Vector2d expected_result4(5.0, 0.0);

    XYZCoord arbitrary(12, 156, 100, 1.3);
    Eigen::Vector2d result5 = dubins1.findCenter(arbitrary, 'L');
    // [-4.817, 1.341] ==> magnitude 5 * e1 vector rotated 2.87 [1.3 + pi/2] raidans
    Eigen::Vector2d expected_result5(12 - 4.817, 156 + 1.341);
    Eigen::Vector2d result6 = dubins1.findCenter(arbitrary, 'R');
    Eigen::Vector2d expected_result6(12 + 4.817, 156 - 1.341);

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
 */
TEST(DubinsTest, CircleArc)
{
    Dubins dubins1(5, 10);

    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord origin_y(0, 0, 0, M_PI / 2);
    XYZCoord arbitrary_position1(73, 41, 0, 4.00);

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

    Eigen::Vector2d result3 = dubins1.circleArc(arbitrary_position1, 1,
                                                dubins1.findCenter(arbitrary_position1, 'L'), 5.12 * 5);
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
 *      fails at last turn
 */
TEST(DubinsTest, GenPointsStraight)
{
    Dubins dubins1(5, 1);
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord arbitrary_position1(9, 6, 0, 4.00);

    // lsl  origin_x ==> arbitrary_position
    DubinsPath path(6.107586558274035, 4.175598748905551, 12.983673916464376);

    std::vector<Eigen::Vector2d> result1 = dubins1.generatePointsStraight(origin_x, arbitrary_position1, path);
    std::vector<Eigen::Vector2d> expected_result1 = {
        Eigen::Vector2d(6.123233995736766e-16, 0.0),
        Eigen::Vector2d(0.9933466539753065, 0.09966711079379209),
        Eigen::Vector2d(1.9470917115432524, 0.3946950299855745),
        Eigen::Vector2d(2.823212366975177, 0.8733219254516085),
        Eigen::Vector2d(3.5867804544976143, 1.5164664532641732),
        Eigen::Vector2d(4.207354924039483, 2.2984884706593016),
        Eigen::Vector2d(4.660195429836132, 3.188211227616632),
        Eigen::Vector2d(4.9272486499423005, 4.1501642854987955),
        Eigen::Vector2d(4.997868015207525, 5.145997611506444),
        Eigen::Vector2d(4.869238154390976, 6.136010473465436),
        Eigen::Vector2d(4.546487134128409, 7.080734182735712),
        Eigen::Vector2d(4.0424820190979505, 7.942505586276729),
        Eigen::Vector2d(3.3773159027557553, 8.686968577706228),
        Eigen::Vector2d(2.5775068591073205, 9.284443766844737),
        Eigen::Vector2d(1.6749407507795255, 9.71111170334329),
        Eigen::Vector2d(0.7056000402993361, 9.949962483002228),
        Eigen::Vector2d(-0.2918707171379004, 9.991473878973766),
        Eigen::Vector2d(-1.2777055101341561, 9.833990962897305),
        Eigen::Vector2d(-2.212602216474262, 9.483792081670735),
        Eigen::Vector2d(-3.059289454713595, 8.954838559572083),
        Eigen::Vector2d(-3.7840124765396412, 8.26821810431806),
        Eigen::Vector2d(-4.357878862067941, 7.451304106703497),
        Eigen::Vector2d(-4.758010369447581, 6.536664349892097),
        Eigen::Vector2d(-4.968455018167322, 5.5607626346752745),
        Eigen::Vector2d(-4.980823044179203, 4.562505082802768),
        Eigen::Vector2d(-4.794621373315692, 3.5816890726838686),
        Eigen::Vector2d(-4.417273278600765, 2.6574166434981144),
        Eigen::Vector2d(-3.8638224377799353, 1.8265356202868266),
        Eigen::Vector2d(-3.156333189361607, 1.1221706074487519),
        Eigen::Vector2d(-2.323010897068786, 0.5724024152934053),
        Eigen::Vector2d(-1.3970774909946289, 0.1991485667481694),
        Eigen::Vector2d(-0.4185269590432342, -0.0038326600561875424),
        Eigen::Vector2d(0.5660951562987457, -0.1785303703430865),
        Eigen::Vector2d(1.5507172716407256, -0.35322808062998534),
        Eigen::Vector2d(2.535339386982705, -0.5279257909168843),
        Eigen::Vector2d(3.519961502324685, -0.7026235012037831),
        Eigen::Vector2d(4.504583617666665, -0.8773212114906821),
        Eigen::Vector2d(5.489205733008644, -1.052018921777581),
        Eigen::Vector2d(6.473827848350624, -1.22671663206448),
        Eigen::Vector2d(7.458449963692604, -1.401414342351379),
        Eigen::Vector2d(8.443072079034584, -1.5761120526382777),
        Eigen::Vector2d(9.427694194376564, -1.7508097629251764),
        Eigen::Vector2d(10.412316309718543, -1.9255074732120754),
        Eigen::Vector2d(11.396938425060522, -2.1002051834989746),
        Eigen::Vector2d(12.384837277736102, -2.2522585496455405),
        Eigen::Vector2d(13.382970185417884, -2.2322134370087867),
        Eigen::Vector2d(14.357224539367046, -2.014269495250773),
        Eigen::Vector2d(15.26875989292903, -1.6071154615675085),
        Eigen::Vector2d(16.081236208073463, -1.0269832824330973),
        Eigen::Vector2d(16.762262618032818, -0.2970009971166694),
        Eigen::Vector2d(17.284688748945122, 0.5537293042585145),
        Eigen::Vector2d(17.6276871195819, 1.491291689211344),
        Eigen::Vector2d(17.777583467299806, 2.478308504102941),
        Eigen::Vector2d(17.728401897740607, 3.4754305032352506),
        Eigen::Vector2d(17.482103124881213, 4.442905579103292),
        Eigen::Vector2d(17.048506303554806, 5.342163553476361),
        Eigen::Vector2d(16.444897570733772, 6.1373538486888375),
        Eigen::Vector2d(15.695340901798186, 6.796774737038687),
        Eigen::Vector2d(14.829718755775655, 7.294137188630348),
        Eigen::Vector2d(13.882540755999456, 7.609612932036834),
        Eigen::Vector2d(12.89156790032764, 7.73062494490981),
        Eigen::Vector2d(11.896307149320169, 7.652348860171518),
        Eigen::Vector2d(10.936436408392781, 7.377905298306002),
        Eigen::Vector2d(10.050222694938975, 6.918235458068104),
        Eigen::Vector2d(9.272996553105063, 6.2916649254160415),
        Eigen::Vector2d(9.0, 6.0)};

    EXPECT_EQ(result1.size(), expected_result1.size());

    // could throw an indexOutOfBounds error
    for (int i = 0; i < result1.size(); i++)
    {
        EXPECT_NEAR(result1[i][0], expected_result1[i][0], 0.01);
        EXPECT_NEAR(result1[i][1], expected_result1[i][1], 0.01);
    }
}

/*
 *   tests Dubins::generatePointsCurve()
 */
TEST(DubinsTest, GenPointsCurve)
{
    Dubins dubins1(5, 1);
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord arbitrary_position1(9, 6, 0, 4.00);

    // lrl  origin_x ==> arbitrary_position
    DubinsPath path(2.25948315258286, 0.3274953432143759, 4.870163802976823);

    std::vector<Eigen::Vector2d> result1 = dubins1.generatePointsCurve(origin_x, arbitrary_position1, path);
    std::vector<Eigen::Vector2d> expected_result1 = {
        Eigen::Vector2d(6.123233995736766e-16, 0.0),
        Eigen::Vector2d(0.9933466539753065, 0.09966711079379209),
        Eigen::Vector2d(1.9470917115432524, 0.3946950299855745),
        Eigen::Vector2d(2.823212366975177, 0.8733219254516085),
        Eigen::Vector2d(3.5867804544976143, 1.5164664532641732),
        Eigen::Vector2d(4.207354924039483, 2.2984884706593016),
        Eigen::Vector2d(4.660195429836132, 3.188211227616632),
        Eigen::Vector2d(4.9272486499423005, 4.1501642854987955),
        Eigen::Vector2d(4.997868015207525, 5.145997611506444),
        Eigen::Vector2d(4.869238154390976, 6.136010473465436),
        Eigen::Vector2d(4.546487134128409, 7.080734182735712),
        Eigen::Vector2d(4.0424820190979505, 7.942505586276729),
        Eigen::Vector2d(3.453414225327183, 8.749607451779864),
        Eigen::Vector2d(3.0208190902513206, 9.649347723773186),
        Eigen::Vector2d(2.7755978543530304, 10.617096479082347),
        Eigen::Vector2d(2.727526714467218, 11.61427262874097),
        Eigen::Vector2d(2.878522115243877, 12.601121906433475),
        Eigen::Vector2d(3.222564346547582, 13.538301745641235),
        Eigen::Vector2d(3.7459375303042997, 14.388449743617045),
        Eigen::Vector2d(4.427776429277235, 15.11767318247875),
        Eigen::Vector2d(5.240898278231409, 15.69690022491657),
        Eigen::Vector2d(6.1528864750033945, 16.10303891660514),
        Eigen::Vector2d(7.127382928133069, 16.31989778955159),
        Eigen::Vector2d(8.125537539235536, 16.338831364829296),
        Eigen::Vector2d(9.107557033825168, 16.15908482054028),
        Eigen::Vector2d(10.03429139359439, 15.787824084182331),
        Eigen::Vector2d(10.868794644098593, 15.239850149733034),
        Eigen::Vector2d(11.57779777416746, 14.537009008727138),
        Eigen::Vector2d(12.133035066393902, 13.707320719513074),
        Eigen::Vector2d(12.512370962089452, 12.783862335949),
        Eigen::Vector2d(12.700682536156371, 11.803449229646171),
        Eigen::Vector2d(12.690462400388142, 10.805167377280418),
        Eigen::Vector2d(12.48211799934634, 9.828815126045004),
        Eigen::Vector2d(12.08395536683173, 8.913316559139),
        Eigen::Vector2d(11.511847990527825, 8.095169715402147),
        Eigen::Vector2d(10.788603986138625, 7.4069915276825675),
        Eigen::Vector2d(9.955581925670863, 6.854885170996804),
        Eigen::Vector2d(9.192828057382332, 6.210775208064388),
        Eigen::Vector2d(9.0, 6.0)};

    EXPECT_EQ(result1.size(), expected_result1.size());

    // could throw an indexOutOfBounds error
    for (int i = 0; i < result1.size(); i++)
    {
        EXPECT_NEAR(result1[i][0], expected_result1[i][0], 0.01);
        EXPECT_NEAR(result1[i][1], expected_result1[i][1], 0.01);
    }
}

/*
 *   tests Dubins::generatePoints()
 */
TEST(DubinsTest, GenPoints)
{
    Dubins dubins1(5, 1);
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord arbitrary_position1(9, 6, 0, 4.00);

    // straight path
    RRTOption lsl(64.39960045236231, DubinsPath(6.107586558274035, 4.175598748905551, 12.983673916464376), true);
    // curve only path
    RRTOption lrl(37.28571149387029, DubinsPath(2.25948315258286, 0.3274953432143759, -4.870163802976823), false);

    std::vector<Eigen::Vector2d> result1 = dubins1.generatePoints(origin_x, arbitrary_position1, lsl.dubins_path, lsl.has_straight);
    std::vector<Eigen::Vector2d> result2 = dubins1.generatePoints(origin_x, arbitrary_position1, lrl.dubins_path, lrl.has_straight);

    std::vector<Eigen::Vector2d> expected_result1 = {
        Eigen::Vector2d(6.123233995736766e-16, 0.0),
        Eigen::Vector2d(0.9933466539753065, 0.09966711079379209),
        Eigen::Vector2d(1.9470917115432524, 0.3946950299855745),
        Eigen::Vector2d(2.823212366975177, 0.8733219254516085),
        Eigen::Vector2d(3.5867804544976143, 1.5164664532641732),
        Eigen::Vector2d(4.207354924039483, 2.2984884706593016),
        Eigen::Vector2d(4.660195429836132, 3.188211227616632),
        Eigen::Vector2d(4.9272486499423005, 4.1501642854987955),
        Eigen::Vector2d(4.997868015207525, 5.145997611506444),
        Eigen::Vector2d(4.869238154390976, 6.136010473465436),
        Eigen::Vector2d(4.546487134128409, 7.080734182735712),
        Eigen::Vector2d(4.0424820190979505, 7.942505586276729),
        Eigen::Vector2d(3.3773159027557553, 8.686968577706228),
        Eigen::Vector2d(2.5775068591073205, 9.284443766844737),
        Eigen::Vector2d(1.6749407507795255, 9.71111170334329),
        Eigen::Vector2d(0.7056000402993361, 9.949962483002228),
        Eigen::Vector2d(-0.2918707171379004, 9.991473878973766),
        Eigen::Vector2d(-1.2777055101341561, 9.833990962897305),
        Eigen::Vector2d(-2.212602216474262, 9.483792081670735),
        Eigen::Vector2d(-3.059289454713595, 8.954838559572083),
        Eigen::Vector2d(-3.7840124765396412, 8.26821810431806),
        Eigen::Vector2d(-4.357878862067941, 7.451304106703497),
        Eigen::Vector2d(-4.758010369447581, 6.536664349892097),
        Eigen::Vector2d(-4.968455018167322, 5.5607626346752745),
        Eigen::Vector2d(-4.980823044179203, 4.562505082802768),
        Eigen::Vector2d(-4.794621373315692, 3.5816890726838686),
        Eigen::Vector2d(-4.417273278600765, 2.6574166434981144),
        Eigen::Vector2d(-3.8638224377799353, 1.8265356202868266),
        Eigen::Vector2d(-3.156333189361607, 1.1221706074487519),
        Eigen::Vector2d(-2.323010897068786, 0.5724024152934053),
        Eigen::Vector2d(-1.3970774909946289, 0.1991485667481694),
        Eigen::Vector2d(-0.4185269590432342, -0.0038326600561875424),
        Eigen::Vector2d(0.5660951562987457, -0.1785303703430865),
        Eigen::Vector2d(1.5507172716407256, -0.35322808062998534),
        Eigen::Vector2d(2.535339386982705, -0.5279257909168843),
        Eigen::Vector2d(3.519961502324685, -0.7026235012037831),
        Eigen::Vector2d(4.504583617666665, -0.8773212114906821),
        Eigen::Vector2d(5.489205733008644, -1.052018921777581),
        Eigen::Vector2d(6.473827848350624, -1.22671663206448),
        Eigen::Vector2d(7.458449963692604, -1.401414342351379),
        Eigen::Vector2d(8.443072079034584, -1.5761120526382777),
        Eigen::Vector2d(9.427694194376564, -1.7508097629251764),
        Eigen::Vector2d(10.412316309718543, -1.9255074732120754),
        Eigen::Vector2d(11.396938425060522, -2.1002051834989746),
        Eigen::Vector2d(12.384837277736102, -2.2522585496455405),
        Eigen::Vector2d(13.382970185417884, -2.2322134370087867),
        Eigen::Vector2d(14.357224539367046, -2.014269495250773),
        Eigen::Vector2d(15.26875989292903, -1.6071154615675085),
        Eigen::Vector2d(16.081236208073463, -1.0269832824330973),
        Eigen::Vector2d(16.762262618032818, -0.2970009971166694),
        Eigen::Vector2d(17.284688748945122, 0.5537293042585145),
        Eigen::Vector2d(17.6276871195819, 1.491291689211344),
        Eigen::Vector2d(17.777583467299806, 2.478308504102941),
        Eigen::Vector2d(17.728401897740607, 3.4754305032352506),
        Eigen::Vector2d(17.482103124881213, 4.442905579103292),
        Eigen::Vector2d(17.048506303554806, 5.342163553476361),
        Eigen::Vector2d(16.444897570733772, 6.1373538486888375),
        Eigen::Vector2d(15.695340901798186, 6.796774737038687),
        Eigen::Vector2d(14.829718755775655, 7.294137188630348),
        Eigen::Vector2d(13.882540755999456, 7.609612932036834),
        Eigen::Vector2d(12.89156790032764, 7.73062494490981),
        Eigen::Vector2d(11.896307149320169, 7.652348860171518),
        Eigen::Vector2d(10.936436408392781, 7.377905298306002),
        Eigen::Vector2d(10.050222694938975, 6.918235458068104),
        Eigen::Vector2d(9.272996553105063, 6.2916649254160415),
        Eigen::Vector2d(9.0, 6.0)};

    std::vector<Eigen::Vector2d> expected_result2 = {
        Eigen::Vector2d(6.123233995736766e-16, 0.0),
        Eigen::Vector2d(0.9933466539753065, 0.09966711079379209),
        Eigen::Vector2d(1.9470917115432524, 0.3946950299855745),
        Eigen::Vector2d(2.823212366975177, 0.8733219254516085),
        Eigen::Vector2d(3.5867804544976143, 1.5164664532641732),
        Eigen::Vector2d(4.207354924039483, 2.2984884706593016),
        Eigen::Vector2d(4.660195429836132, 3.188211227616632),
        Eigen::Vector2d(4.9272486499423005, 4.1501642854987955),
        Eigen::Vector2d(4.997868015207525, 5.145997611506444),
        Eigen::Vector2d(4.869238154390976, 6.136010473465436),
        Eigen::Vector2d(4.546487134128409, 7.080734182735712),
        Eigen::Vector2d(4.0424820190979505, 7.942505586276729),
        Eigen::Vector2d(3.453414225327183, 8.749607451779864),
        Eigen::Vector2d(3.0208190902513206, 9.649347723773186),
        Eigen::Vector2d(2.7755978543530304, 10.617096479082347),
        Eigen::Vector2d(2.727526714467218, 11.61427262874097),
        Eigen::Vector2d(2.878522115243877, 12.601121906433475),
        Eigen::Vector2d(3.222564346547582, 13.538301745641235),
        Eigen::Vector2d(3.7459375303042997, 14.388449743617045),
        Eigen::Vector2d(4.427776429277235, 15.11767318247875),
        Eigen::Vector2d(5.240898278231409, 15.69690022491657),
        Eigen::Vector2d(6.1528864750033945, 16.10303891660514),
        Eigen::Vector2d(7.127382928133069, 16.31989778955159),
        Eigen::Vector2d(8.125537539235536, 16.338831364829296),
        Eigen::Vector2d(9.107557033825168, 16.15908482054028),
        Eigen::Vector2d(10.03429139359439, 15.787824084182331),
        Eigen::Vector2d(10.868794644098593, 15.239850149733034),
        Eigen::Vector2d(11.57779777416746, 14.537009008727138),
        Eigen::Vector2d(12.133035066393902, 13.707320719513074),
        Eigen::Vector2d(12.512370962089452, 12.783862335949),
        Eigen::Vector2d(12.700682536156371, 11.803449229646171),
        Eigen::Vector2d(12.690462400388142, 10.805167377280418),
        Eigen::Vector2d(12.48211799934634, 9.828815126045004),
        Eigen::Vector2d(12.08395536683173, 8.913316559139),
        Eigen::Vector2d(11.511847990527825, 8.095169715402147),
        Eigen::Vector2d(10.788603986138625, 7.4069915276825675),
        Eigen::Vector2d(9.955581925670863, 6.854885170996804),
        Eigen::Vector2d(9.192828057382332, 6.210775208064388),
        Eigen::Vector2d(9.0, 6.0)};

    EXPECT_EQ(result1.size(), expected_result1.size());
    EXPECT_EQ(result2.size(), expected_result2.size());

    // could throw an indexOutOfBounds error
    for (int i = 0; i < result1.size(); i++)
    {
        EXPECT_NEAR(result1[i][0], expected_result1[i][0], 0.01);
        EXPECT_NEAR(result1[i][1], expected_result1[i][1], 0.01);
    }

    // could throw an indexOutOfBounds error
    for (int i = 0; i < result2.size(); i++)
    {
        EXPECT_NEAR(result2[i][0], expected_result2[i][0], 0.01);
        EXPECT_NEAR(result2[i][1], expected_result2[i][1], 0.01);
    }
}

/*
 *   tests Dubins::lsl()
 */
TEST(DubinsTest, LSL)
{
    Dubins dubins1(5, 10);

    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord arbitrary_position1(73, 41, 0, 4.00);
    XYZCoord plus_x100(100, 0, 0, 0);
    XYZCoord arbitrary_position2(5, 100, 0, M_PI / 2);

    RRTOption result1 = dubins1.lsl(origin_x, arbitrary_position1,
                                    dubins1.findCenter(origin_x, 'L'), dubins1.findCenter(arbitrary_position1, 'L'));
    RRTOption expected_result1(103.46948015930067, DubinsPath(0.40295754, 3.5970424510, 83.46948015930067), true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = dubins1.lsl(origin_x, plus_x100,
                                    dubins1.findCenter(origin_x, 'L'), dubins1.findCenter(plus_x100, 'L'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 = dubins1.lsl(origin_x, arbitrary_position2,
                                    dubins1.findCenter(origin_x, 'L'), dubins1.findCenter(arbitrary_position2, 'L'));
    RRTOption expected_result3(102.85398163397448, DubinsPath(M_PI / 2, 0, 95), true);

    EXPECT_NEAR(result3.length, expected_result3.length, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_0, expected_result3.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_2, expected_result3.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result3.dubins_path.straight_dist, result3.dubins_path.straight_dist, 0.01);
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
    XYZCoord arbitrary_position1(73, 41, 0, 4.00);
    XYZCoord plus_x100(100, 0, 0, 0);
    XYZCoord arbitrary_position2(5, -100, 0, -M_PI / 2);

    RRTOption result1 = dubins1.rsr(origin_x, arbitrary_position1,
                                    dubins1.findCenter(origin_x, 'R'), dubins1.findCenter(arbitrary_position1, 'R'));
    RRTOption expected_result1(127.792, DubinsPath(-5.664581035483313, -2.9017895788758596, 84.96005087111514), true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = dubins1.rsr(origin_x, plus_x100,
                                    dubins1.findCenter(origin_x, 'R'), dubins1.findCenter(plus_x100, 'R'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 = dubins1.rsr(origin_x, arbitrary_position2,
                                    dubins1.findCenter(origin_x, 'R'), dubins1.findCenter(arbitrary_position2, 'R'));
    RRTOption expected_result3(102.85398163397448, DubinsPath(-M_PI / 2, 0, 95), true);

    EXPECT_NEAR(result3.length, expected_result3.length, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_0, expected_result3.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_2, expected_result3.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result3.dubins_path.straight_dist, result3.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result3.has_straight, expected_result3.has_straight);
}

/*
 *   tests Dubins::rsl()
 */
TEST(DubinsTest, RSL)
{
    Dubins dubins1(5, 10);
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord arbitrary_position1(73, 41, 0, 4.00);
    XYZCoord plus_x100(100, 0, 0, 0);
    XYZCoord arbitrary_position2(10, -100, 0, 0);

    RRTOption result1 = dubins1.rsl(origin_x, arbitrary_position1,
                                    dubins1.findCenter(origin_x, 'R'), dubins1.findCenter(arbitrary_position1, 'L'));
    RRTOption expected_result1(134.78090998278276, DubinsPath(-5.8893974274779834, 3.606212120298397, 87.30286224390085), true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = dubins1.rsl(origin_x, plus_x100,
                                    dubins1.findCenter(origin_x, 'R'), dubins1.findCenter(plus_x100, 'L'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 = dubins1.rsl(origin_x, arbitrary_position2,
                                    dubins1.findCenter(origin_x, 'R'), dubins1.findCenter(arbitrary_position2, 'L'));
    RRTOption expected_result3(105.70796326794898, DubinsPath(-M_PI / 2, M_PI / 2, 90), true);

    EXPECT_NEAR(result3.length, expected_result3.length, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_0, expected_result3.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_2, expected_result3.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result3.dubins_path.straight_dist, result3.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result3.has_straight, expected_result3.has_straight);
}

/*
 *   tests Dubins::lsr()
 */
TEST(DubinsTest, LSR)
{
    Dubins dubins1(5, 10);
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord arbitrary_position1(73, 41, 0, 4.00);
    XYZCoord plus_x100(100, 0, 0, 0);
    XYZCoord arbitrary_position2(10, 100, 0, 0);

    RRTOption result1 = dubins1.lsr(origin_x, arbitrary_position1,
                                    dubins1.findCenter(origin_x, 'L'), dubins1.findCenter(arbitrary_position1, 'R'));
    RRTOption expected_result1(96.78474229907584, DubinsPath(0.6420440973470476, -2.925229404526634, 78.94837478970744), true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = dubins1.lsr(origin_x, plus_x100,
                                    dubins1.findCenter(origin_x, 'L'), dubins1.findCenter(plus_x100, 'R'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 = dubins1.lsr(origin_x, arbitrary_position2,
                                    dubins1.findCenter(origin_x, 'L'), dubins1.findCenter(arbitrary_position2, 'R'));
    RRTOption expected_result3(105.70796326794898, DubinsPath(M_PI / 2, -M_PI / 2, 90), true);
    EXPECT_NEAR(result3.length, expected_result3.length, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_0, expected_result3.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_2, expected_result3.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result3.dubins_path.straight_dist, result3.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result3.has_straight, expected_result3.has_straight);
}

/*
 *   tests Dubins::lrl()
 */
TEST(DubinsTest, LRL)
{
    Dubins dubins1(5, 10);
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord arbitrary_position1(9, 6, 0, 4.00);

    RRTOption result1 = dubins1.lrl(origin_x, arbitrary_position1,
                                    dubins1.findCenter(origin_x, 'L'), dubins1.findCenter(arbitrary_position1, 'L'));
    RRTOption expected_result1(37.28571149387029, DubinsPath(2.25948315258286, 0.3274953432143759, 4.870163802976823), false);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);
}

/*
 *   tests Dubins::rlr()
 */
TEST(DubinsTest, RLR)
{
    Dubins dubins1(5, 10);
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord arbitrary_position1(9, 6, 0, 4.00);

    RRTOption result1 = dubins1.rlr(origin_x, arbitrary_position1,
                                    dubins1.findCenter(origin_x, 'R'), dubins1.findCenter(arbitrary_position1, 'R'));
    RRTOption expected_result1(56.99424154724155, DubinsPath(-1.0585943958426456, -5.782422412471302, 4.557831501134362), false);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);
}

/*
 *   tests Dubins::allOptions()
 */
TEST(DubinsTest, AllOptions)
{
    Dubins dubins1(5, 10);
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord arbitrary_position1(9, 6, 0, 4.00);

    std::vector<RRTOption> result1 = dubins1.allOptions(origin_x, arbitrary_position1);
    std::vector<RRTOption> expected_result1 = {
        RRTOption(64.39960045236231, DubinsPath(6.107586558274035, 4.175598748905551, 12.983673916464376), true),
        RRTOption(58.0235802190719, DubinsPath(-5.062863952455051, -3.5035066619041215, 15.191727147276039), true),
        RRTOption(32.99374082753003, DubinsPath(-0.18936765807467593, 4.189367658074676, 11.100064246783269), true),
        RRTOption(std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), true),
        RRTOption(56.99424154724155, DubinsPath(-1.0585943958426456, -5.782422412471302, 4.557831501134362), false),
        RRTOption(37.28571149387029, DubinsPath(2.25948315258286, 0.3274953432143759, -4.870163802976823), false),
    };

    for (int i = 0; i < result1.size(); i++)
    {
        // if the path is impossible or provablly non-competitive (length == inf),
        // then checking inf is unique, and nothing else needs to be checked (should be garbage data)
        if (std::isinf(result1[i].length) || std::isinf(expected_result1[i].length))
        {
            EXPECT_EQ(std::isinf(result1[i].length), std::isinf(expected_result1[i].length));
        }
        else
        {
            EXPECT_NEAR(result1[i].length, expected_result1[i].length, 0.01);
            EXPECT_NEAR(result1[i].dubins_path.beta_0, expected_result1[i].dubins_path.beta_0, 0.01);
            EXPECT_NEAR(result1[i].dubins_path.beta_2, expected_result1[i].dubins_path.beta_2, 0.01);
            EXPECT_NEAR(result1[i].dubins_path.straight_dist, expected_result1[i].dubins_path.straight_dist, 0.01);
        }

        EXPECT_EQ(result1[i].has_straight, expected_result1[i].has_straight);
    }

    EXPECT_EQ(5, 5);
}

/*
 *   tests Dubins::dubinsPath()
 */
TEST(DubinsTest, DubinsPath)
{
    Dubins dubins1(5, 1);
    // points towards e1
    XYZCoord origin_x(0, 0, 0, 0);
    XYZCoord arbitrary_position1(9, 6, 0, 4.00);

    std::vector<Eigen::Vector2d> result1 = dubins1.dubinsPath(origin_x, arbitrary_position1);
    std::vector<Eigen::Vector2d> expected_result1 = {
        Eigen::Vector2d(6.123233995736766e-16, 0.0),
        Eigen::Vector2d(0.993400836368525, -0.09938973742323216),
        Eigen::Vector2d(1.975524298544812, -0.2876276322341959),
        Eigen::Vector2d(2.957647760721099, -0.4758655270451597),
        Eigen::Vector2d(3.939771222897386, -0.6641034218561235),
        Eigen::Vector2d(4.9218946850736724, -0.8523413166670872),
        Eigen::Vector2d(5.904018147249959, -1.040579211478051),
        Eigen::Vector2d(6.886141609426246, -1.2288171062890147),
        Eigen::Vector2d(7.868265071602533, -1.4170550010999785),
        Eigen::Vector2d(8.850388533778819, -1.6052928959109423),
        Eigen::Vector2d(9.832511995955107, -1.793530790721906),
        Eigen::Vector2d(10.814635458131391, -1.9817686855328698),
        Eigen::Vector2d(11.79675892030768, -2.1700065803438333),
        Eigen::Vector2d(12.79027164737483, -2.26821418659457),
        Eigen::Vector2d(13.783492756324007, -2.1673036486132564),
        Eigen::Vector2d(14.736867740560596, -1.8710820299393291),
        Eigen::Vector2d(15.61238854801182, -1.3913587517279438),
        Eigen::Vector2d(16.375150926950273, -0.7472588672270439),
        Eigen::Vector2d(16.994745948363555, 0.035539393743043934),
        Eigen::Vector2d(17.446472313993077, 0.9258283347801974),
        Eigen::Vector2d(17.712321119146168, 1.8881149452744896),
        Eigen::Vector2d(17.781693810895792, 2.8840358947365097),
        Eigen::Vector2d(17.65182471894008, 3.873886957721502),
        Eigen::Vector2d(17.32789131414969, 4.818205896003914),
        Eigen::Vector2d(16.822807799142637, 5.67934569348516),
        Eigen::Vector2d(16.15671025977906, 6.422975423923436),
        Eigen::Vector2d(15.356153902961042, 7.019448916625484),
        Eigen::Vector2d(14.453054384333244, 7.444986655718225),
        Eigen::Vector2d(13.48341543180803, 7.68262379440602),
        Eigen::Vector2d(12.48589349054992, 7.722886489876743),
        Eigen::Vector2d(11.500256612494358, 7.56416959551826),
        Eigen::Vector2d(10.565799029612386, 7.212800653048518),
        Eigen::Vector2d(9.719774616882088, 6.682787633394971),
        Eigen::Vector2d(9.0, 6.0)};

    EXPECT_EQ(result1.size(), expected_result1.size());

    for (int i = 0; i < result1.size(); i++)
    {
        EXPECT_NEAR(result1[i][0], expected_result1[i][0], 0.01);
        EXPECT_NEAR(result1[i][1], expected_result1[i][1], 0.01);
    }
}