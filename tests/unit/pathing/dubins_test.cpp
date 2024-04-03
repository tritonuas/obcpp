#include "pathing/dubins.hpp"

#include <gtest/gtest.h>

#include <cmath>

#include "utilities/datatypes.hpp"

typedef XYZCoord Vector;

/*
 *   NOTE: the use of () and {} constructors is non-staandard
 *   i.e. I originally wrote it using () and was too lazy to
 *   convert all of it to {}
 */

/*
 *   Tests dubins ==> sign()
 */
TEST(DubinsUtilTest, Sign) {
    EXPECT_EQ(sign(99.0), 1.0);
    EXPECT_EQ(sign(0.0), 0.0);
    EXPECT_EQ(sign(-50.0), -1.0);
}

/*
 *   Tests dubins ==> mod()
 */
TEST(DubinsUtilTest, Modulo) {
    EXPECT_NEAR(mod(95.7, 10.5), 1.2, 0.001);
    EXPECT_NEAR(mod(0.0, 10.0), 0.0, 0.001);
    EXPECT_NEAR(mod(-1.0, 10.0), 9, 0.001);
    EXPECT_NEAR(mod(5.0, 5.0), 0.0, 0.001);
    EXPECT_NEAR(mod(-5.0, -5.0), 0.0, 0.001);
}

/*
 *   Tests dubins ==> compareRRTLength()
 */
TEST(DubinsUtilTest, CompareRRT) {
    RRTOption infinite_length =
        RRTOption{std::numeric_limits<double>::infinity(), DubinsPath{0, 0, 0}, true};
    RRTOption long_length = RRTOption{99999, DubinsPath{0, 0, 0}, true};
    RRTOption short_length = RRTOption{10, DubinsPath{0, 0, 9999}, false};

    EXPECT_EQ(compareRRTOptionLength(infinite_length, long_length), false);
    EXPECT_EQ(compareRRTOptionLength(short_length, long_length), true);
    EXPECT_EQ(compareRRTOptionLength(long_length, short_length), false);

    // not needed for the behavior of the function, but testing predictable implementation
    EXPECT_EQ(compareRRTOptionLength(short_length, short_length), false);
    EXPECT_EQ(compareRRTOptionLength(infinite_length, infinite_length), false);
}

/*
 *   Tests dubins ==> findOrthogonalVector2D()
 */
TEST(DubinsUtilTest, Orthogonal2D) {
    Vector input_vector1{1.0, 2.0, 0};
    Vector input_vector2{-3.0, 4.0, 0};
    Vector input_vector3{0.0, 0.0, 0};  // origin
    Vector input_vector4{1.0, 0.0, 0};  // e1 basis vector
    Vector input_vector5{-1.0, -1.0, 0};

    Vector expected_output1{-2.0, 1.0, 0};
    Vector expected_output2{-4.0, -3.0, 0};
    Vector expected_output3{0.0, 0.0, 0};  // should not change origin
    Vector expected_output4{0.0, 1.0, 0};  // e2 basis vector (90 CCW rotation)
    Vector expected_output5(1.0, -1.0, 0);

    EXPECT_EQ(findOrthogonalVector2D(input_vector1), expected_output1);
    EXPECT_EQ(findOrthogonalVector2D(input_vector2), expected_output2);
    EXPECT_EQ(findOrthogonalVector2D(input_vector3), expected_output3);
    EXPECT_EQ(findOrthogonalVector2D(input_vector4), expected_output4);
    EXPECT_EQ(findOrthogonalVector2D(input_vector5), expected_output5);
}

/*
 *   tests dubins ==> halfDisplacement()
 */
TEST(DubinsUtilTest, HalfDisplacement) {
    // 1] Results with Integer components
    Vector start_vector1{0.0, 0.0, 0};
    Vector end_vector1{0.0, 0.0, 0};
    Vector half_displacement1{0.0, 0.0, 0};

    Vector start_vector2{2.0, 0.0, 0};
    Vector end_vector2{0.0, 2.0, 0};
    Vector half_displacement2{1.0, -1.0, 0};

    Vector start_vector3{2.0, 0.0, 0};
    Vector end_vector3{6.0, 0.0, 0};
    Vector half_displacement3{-2.0, 0.0, 0};

    EXPECT_EQ(halfDisplacement(start_vector1, end_vector1), half_displacement1);
    EXPECT_EQ(halfDisplacement(start_vector2, end_vector2), half_displacement2);
    EXPECT_EQ(halfDisplacement(start_vector3, end_vector3), half_displacement3);

    // 2] Results without Integer components
    Vector start_vector4{1.0, 0.0, 0};
    Vector end_vector4{0.0, 1.0, 0};
    Vector half_displacement4{0.5, -0.5, 0};

    Vector start_vector5{102.5, -125.5, 0};
    Vector end_vector5{1825.0, 2389.8, 0};
    Vector half_displacement5{-861.25, -1257.65, 0};

    Vector result_vector4 = halfDisplacement(start_vector4, end_vector4);
    Vector result_vector5 = halfDisplacement(start_vector5, end_vector5);

    EXPECT_NEAR(result_vector4.x, half_displacement4.x, 0.01);
    EXPECT_NEAR(result_vector4.y, half_displacement4.y, 0.01);
    EXPECT_NEAR(result_vector5.x, half_displacement5.x, 0.01);
    EXPECT_NEAR(result_vector5.y, half_displacement5.y, 0.01);
}

/*
 *   tests Dubins::findCenter()
 */
TEST(DubinsTest, FindCenter) {
    Dubins dubins1(5, 10);

    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    Vector result1 = dubins1.findCenter(origin_x, 'L');
    Vector expected_result1{0.0, 5.0, 0};
    Vector result2 = dubins1.findCenter(origin_x, 'R');
    Vector expected_result2{0.0, -5.0, 0};

    // points towards e2
    RRTPoint origin_y{Vector{0, 0, 0}, M_PI / 2};
    Vector result3 = dubins1.findCenter(origin_y, 'L');
    Vector expected_result3{-5.0, 0.0, 0};
    Vector result4 = dubins1.findCenter(origin_y, 'R');
    Vector expected_result4{5.0, 0.0, 0};

    RRTPoint arbitrary{Vector{12, 156, 100}, 1.3};
    Vector result5 = dubins1.findCenter(arbitrary, 'L');
    // [-4.817, 1.341] ==> magnitude 5 * e1 vector rotated 2.87 [1.3 + pi/2] raidans
    Vector expected_result5{12 - 4.817, 156 + 1.341, 0};
    Vector result6 = dubins1.findCenter(arbitrary, 'R');
    Vector expected_result6{12 + 4.817, 156 - 1.341, 0};

    EXPECT_NEAR(result1.x, expected_result1.x, 0.01);
    EXPECT_NEAR(result1.y, expected_result1.y, 0.01);

    EXPECT_NEAR(result2.x, expected_result2.x, 0.01);
    EXPECT_NEAR(result2.y, expected_result2.y, 0.01);

    EXPECT_NEAR(result3.x, expected_result3.x, 0.01);
    EXPECT_NEAR(result3.y, expected_result3.y, 0.01);

    EXPECT_NEAR(result4.x, expected_result4.x, 0.01);
    EXPECT_NEAR(result4.y, expected_result4.y, 0.01);

    EXPECT_NEAR(result5.x, expected_result5.x, 0.01);
    EXPECT_NEAR(result5.y, expected_result5.y, 0.01);

    EXPECT_NEAR(result6.x, expected_result6.x, 0.01);
    EXPECT_NEAR(result6.y, expected_result6.y, 0.01);
}

/*
 *   tests Dubins::circleArc()
 *   (poorly)
 *
 */
TEST(DubinsTest, CircleArc) {
    Dubins dubins1(5, 10);

    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint origin_y{Vector{0, 0, 0}, M_PI / 2};
    RRTPoint arbitrary_position1{Vector{73, 41, 0}, 4.00};

    // plane is facing x+, turning left/ccw with a turning radius of 5,
    // this should be the point where it turns 90deg (1/4 of the circle)
    Vector result1 =
        dubins1.circleArc(origin_x, 1, dubins1.findCenter(origin_x, 'L'), M_PI / 2 * 5);
    Vector expected_result1{5.0, 5.0, 0};

    // plance facing x+, turning right/cw with a turning radius of 5
    // turning 2.97 rad
    Vector result2 = dubins1.circleArc(origin_x, -1, dubins1.findCenter(origin_x, 'R'), 2.97 * 5);
    Vector expected_result2{0.850, -9.927, 0};

    Vector result3 = dubins1.circleArc(arbitrary_position1, 1,
                                       dubins1.findCenter(arbitrary_position1, 'L'), 5.12 * 5);
    Vector expected_result3{78.28441936, 42.50134993, 0};

    Vector result4 = dubins1.circleArc(origin_y, 1, dubins1.findCenter(origin_y, 'L'), M_PI * 5);
    Vector expected_result4{-10.0, 0.0, 0};

    EXPECT_NEAR(result1.x, expected_result1.x, 0.01);
    EXPECT_NEAR(result1.y, expected_result1.y, 0.01);

    EXPECT_NEAR(result2.x, expected_result2.x, 0.01);
    EXPECT_NEAR(result2.y, expected_result2.y, 0.01);

    EXPECT_NEAR(result3.x, expected_result3.x, 0.01);
    EXPECT_NEAR(result3.y, expected_result3.y, 0.01);

    EXPECT_NEAR(result4.x, expected_result4.x, 0.01);
    EXPECT_NEAR(result4.y, expected_result4.y, 0.01);
}

/*
 *   tests Dubins::generatePointsStraight()
 *      fails at last turn
 */
TEST(DubinsTest, GenPointsStraight) {
    Dubins dubins1{5, 1};
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    // lsl  origin_x ==> arbitrary_position
    DubinsPath path{6.107586558274035, 4.175598748905551, 12.983673916464376};

    std::vector<Vector> result1 =
        dubins1.generatePointsStraight(origin_x, arbitrary_position1, path);
    std::vector<Vector> expected_result1 = {Vector{6.123233995736766e-16, 0.0, 0},
                                            Vector{0.9933466539753065, 0.09966711079379209, 0},
                                            Vector{1.9470917115432524, 0.3946950299855745, 0},
                                            Vector{2.823212366975177, 0.8733219254516085, 0},
                                            Vector{3.5867804544976143, 1.5164664532641732, 0},
                                            Vector{4.207354924039483, 2.2984884706593016, 0},
                                            Vector{4.660195429836132, 3.188211227616632, 0},
                                            Vector{4.9272486499423005, 4.1501642854987955, 0},
                                            Vector{4.997868015207525, 5.145997611506444, 0},
                                            Vector{4.869238154390976, 6.136010473465436, 0},
                                            Vector{4.546487134128409, 7.080734182735712, 0},
                                            Vector{4.0424820190979505, 7.942505586276729, 0},
                                            Vector{3.3773159027557553, 8.686968577706228, 0},
                                            Vector{2.5775068591073205, 9.284443766844737, 0},
                                            Vector{1.6749407507795255, 9.71111170334329, 0},
                                            Vector{0.7056000402993361, 9.949962483002228, 0},
                                            Vector{-0.2918707171379004, 9.991473878973766, 0},
                                            Vector{-1.2777055101341561, 9.833990962897305, 0},
                                            Vector{-2.212602216474262, 9.483792081670735, 0},
                                            Vector{-3.059289454713595, 8.954838559572083, 0},
                                            Vector{-3.7840124765396412, 8.26821810431806, 0},
                                            Vector{-4.357878862067941, 7.451304106703497, 0},
                                            Vector{-4.758010369447581, 6.536664349892097, 0},
                                            Vector{-4.968455018167322, 5.5607626346752745, 0},
                                            Vector{-4.980823044179203, 4.562505082802768, 0},
                                            Vector{-4.794621373315692, 3.5816890726838686, 0},
                                            Vector{-4.417273278600765, 2.6574166434981144, 0},
                                            Vector{-3.8638224377799353, 1.8265356202868266, 0},
                                            Vector{-3.156333189361607, 1.1221706074487519, 0},
                                            Vector{-2.323010897068786, 0.5724024152934053, 0},
                                            Vector{-1.3970774909946289, 0.1991485667481694, 0},
                                            Vector{-0.4185269590432342, -0.0038326600561875424, 0},
                                            Vector{0.5660951562987457, -0.1785303703430865, 0},
                                            Vector{1.5507172716407256, -0.35322808062998534, 0},
                                            Vector{2.535339386982705, -0.5279257909168843, 0},
                                            Vector{3.519961502324685, -0.7026235012037831, 0},
                                            Vector{4.504583617666665, -0.8773212114906821, 0},
                                            Vector{5.489205733008644, -1.052018921777581, 0},
                                            Vector{6.473827848350624, -1.22671663206448, 0},
                                            Vector{7.458449963692604, -1.401414342351379, 0},
                                            Vector{8.443072079034584, -1.5761120526382777, 0},
                                            Vector{9.427694194376564, -1.7508097629251764, 0},
                                            Vector{10.412316309718543, -1.9255074732120754, 0},
                                            Vector{11.396938425060522, -2.1002051834989746, 0},
                                            Vector{12.384837277736102, -2.2522585496455405, 0},
                                            Vector{13.382970185417884, -2.2322134370087867, 0},
                                            Vector{14.357224539367046, -2.014269495250773, 0},
                                            Vector{15.26875989292903, -1.6071154615675085, 0},
                                            Vector{16.081236208073463, -1.0269832824330973, 0},
                                            Vector{16.762262618032818, -0.2970009971166694, 0},
                                            Vector{17.284688748945122, 0.5537293042585145, 0},
                                            Vector{17.6276871195819, 1.491291689211344, 0},
                                            Vector{17.777583467299806, 2.478308504102941, 0},
                                            Vector{17.728401897740607, 3.4754305032352506, 0},
                                            Vector{17.482103124881213, 4.442905579103292, 0},
                                            Vector{17.048506303554806, 5.342163553476361, 0},
                                            Vector{16.444897570733772, 6.1373538486888375, 0},
                                            Vector{15.695340901798186, 6.796774737038687, 0},
                                            Vector{14.829718755775655, 7.294137188630348, 0},
                                            Vector{13.882540755999456, 7.609612932036834, 0},
                                            Vector{12.89156790032764, 7.73062494490981, 0},
                                            Vector{11.896307149320169, 7.652348860171518, 0},
                                            Vector{10.936436408392781, 7.377905298306002, 0},
                                            Vector{10.050222694938975, 6.918235458068104, 0},
                                            Vector{9.272996553105063, 6.2916649254160415, 0},
                                            Vector{9.0, 6.0, 0}};

    EXPECT_EQ(result1.size(), expected_result1.size());

    // could throw an indexOutOfBounds error
    for (int i = 0; i < result1.size(); i++) {
        EXPECT_NEAR(result1[i].x, expected_result1[i].x, 0.01);
        EXPECT_NEAR(result1[i].y, expected_result1[i].y, 0.01);
    }
}

/*
 *   tests Dubins::generatePointsCurve()
 */
TEST(DubinsTest, GenPointsCurve) {
    Dubins dubins1{5, 1};
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    // lrl  origin_x ==> arbitrary_position
    DubinsPath path{2.25948315258286, 0.3274953432143759, 4.870163802976823};

    std::vector<Vector> result1 = dubins1.generatePointsCurve(origin_x, arbitrary_position1, path);
    std::vector<Vector> expected_result1 = {Vector{6.123233995736766e-16, 0.0, 0},
                                            Vector{0.9933466539753065, 0.09966711079379209, 0},
                                            Vector{1.9470917115432524, 0.3946950299855745, 0},
                                            Vector{2.823212366975177, 0.8733219254516085, 0},
                                            Vector{3.5867804544976143, 1.5164664532641732, 0},
                                            Vector{4.207354924039483, 2.2984884706593016, 0},
                                            Vector{4.660195429836132, 3.188211227616632, 0},
                                            Vector{4.9272486499423005, 4.1501642854987955, 0},
                                            Vector{4.997868015207525, 5.145997611506444, 0},
                                            Vector{4.869238154390976, 6.136010473465436, 0},
                                            Vector{4.546487134128409, 7.080734182735712, 0},
                                            Vector{4.0424820190979505, 7.942505586276729, 0},
                                            Vector{3.453414225327183, 8.749607451779864, 0},
                                            Vector{3.0208190902513206, 9.649347723773186, 0},
                                            Vector{2.7755978543530304, 10.617096479082347, 0},
                                            Vector{2.727526714467218, 11.61427262874097, 0},
                                            Vector{2.878522115243877, 12.601121906433475, 0},
                                            Vector{3.222564346547582, 13.538301745641235, 0},
                                            Vector{3.7459375303042997, 14.388449743617045, 0},
                                            Vector{4.427776429277235, 15.11767318247875, 0},
                                            Vector{5.240898278231409, 15.69690022491657, 0},
                                            Vector{6.1528864750033945, 16.10303891660514, 0},
                                            Vector{7.127382928133069, 16.31989778955159, 0},
                                            Vector{8.125537539235536, 16.338831364829296, 0},
                                            Vector{9.107557033825168, 16.15908482054028, 0},
                                            Vector{10.03429139359439, 15.787824084182331, 0},
                                            Vector{10.868794644098593, 15.239850149733034, 0},
                                            Vector{11.57779777416746, 14.537009008727138, 0},
                                            Vector{12.133035066393902, 13.707320719513074, 0},
                                            Vector{12.512370962089452, 12.783862335949, 0},
                                            Vector{12.700682536156371, 11.803449229646171, 0},
                                            Vector{12.690462400388142, 10.805167377280418, 0},
                                            Vector{12.48211799934634, 9.828815126045004, 0},
                                            Vector{12.08395536683173, 8.913316559139, 0},
                                            Vector{11.511847990527825, 8.095169715402147, 0},
                                            Vector{10.788603986138625, 7.4069915276825675, 0},
                                            Vector{9.955581925670863, 6.854885170996804, 0},
                                            Vector{9.192828057382332, 6.210775208064388, 0},
                                            Vector{9.0, 6.0, 0}};

    EXPECT_EQ(result1.size(), expected_result1.size());

    // could throw an indexOutOfBounds error
    for (int i = 0; i < result1.size(); i++) {
        EXPECT_NEAR(result1[i].x, expected_result1[i].x, 0.01);
        EXPECT_NEAR(result1[i].y, expected_result1[i].y, 0.01);
    }
}

/*
 *   tests Dubins::generatePoints()
 */
TEST(DubinsTest, GenPoints) {
    Dubins dubins1{5, 1};
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    // straight path
    RRTOption lsl{64.39960045236231,
                  DubinsPath{6.107586558274035, 4.175598748905551, 12.983673916464376}, true};
    // curve only path
    RRTOption lrl{37.28571149387029,
                  DubinsPath{2.25948315258286, 0.3274953432143759, -4.870163802976823}, false};

    std::vector<Vector> result1 =
        dubins1.generatePoints(origin_x, arbitrary_position1, lsl.dubins_path, lsl.has_straight);
    std::vector<Vector> result2 =
        dubins1.generatePoints(origin_x, arbitrary_position1, lrl.dubins_path, lrl.has_straight);

    std::vector<Vector> expected_result1 = {Vector{6.123233995736766e-16, 0.0, 0},
                                            Vector{0.9933466539753065, 0.09966711079379209, 0},
                                            Vector{1.9470917115432524, 0.3946950299855745, 0},
                                            Vector{2.823212366975177, 0.8733219254516085, 0},
                                            Vector{3.5867804544976143, 1.5164664532641732, 0},
                                            Vector{4.207354924039483, 2.2984884706593016, 0},
                                            Vector{4.660195429836132, 3.188211227616632, 0},
                                            Vector{4.9272486499423005, 4.1501642854987955, 0},
                                            Vector{4.997868015207525, 5.145997611506444, 0},
                                            Vector{4.869238154390976, 6.136010473465436, 0},
                                            Vector{4.546487134128409, 7.080734182735712, 0},
                                            Vector{4.0424820190979505, 7.942505586276729, 0},
                                            Vector{3.3773159027557553, 8.686968577706228, 0},
                                            Vector{2.5775068591073205, 9.284443766844737, 0},
                                            Vector{1.6749407507795255, 9.71111170334329, 0},
                                            Vector{0.7056000402993361, 9.949962483002228, 0},
                                            Vector{-0.2918707171379004, 9.991473878973766, 0},
                                            Vector{-1.2777055101341561, 9.833990962897305, 0},
                                            Vector{-2.212602216474262, 9.483792081670735, 0},
                                            Vector{-3.059289454713595, 8.954838559572083, 0},
                                            Vector{-3.7840124765396412, 8.26821810431806, 0},
                                            Vector{-4.357878862067941, 7.451304106703497, 0},
                                            Vector{-4.758010369447581, 6.536664349892097, 0},
                                            Vector{-4.968455018167322, 5.5607626346752745, 0},
                                            Vector{-4.980823044179203, 4.562505082802768, 0},
                                            Vector{-4.794621373315692, 3.5816890726838686, 0},
                                            Vector{-4.417273278600765, 2.6574166434981144, 0},
                                            Vector{-3.8638224377799353, 1.8265356202868266, 0},
                                            Vector{-3.156333189361607, 1.1221706074487519, 0},
                                            Vector{-2.323010897068786, 0.5724024152934053, 0},
                                            Vector{-1.3970774909946289, 0.1991485667481694, 0},
                                            Vector{-0.4185269590432342, -0.0038326600561875424, 0},
                                            Vector{0.5660951562987457, -0.1785303703430865, 0},
                                            Vector{1.5507172716407256, -0.35322808062998534, 0},
                                            Vector{2.535339386982705, -0.5279257909168843, 0},
                                            Vector{3.519961502324685, -0.7026235012037831, 0},
                                            Vector{4.504583617666665, -0.8773212114906821, 0},
                                            Vector{5.489205733008644, -1.052018921777581, 0},
                                            Vector{6.473827848350624, -1.22671663206448, 0},
                                            Vector{7.458449963692604, -1.401414342351379, 0},
                                            Vector{8.443072079034584, -1.5761120526382777, 0},
                                            Vector{9.427694194376564, -1.7508097629251764, 0},
                                            Vector{10.412316309718543, -1.9255074732120754, 0},
                                            Vector{11.396938425060522, -2.1002051834989746, 0},
                                            Vector{12.384837277736102, -2.2522585496455405, 0},
                                            Vector{13.382970185417884, -2.2322134370087867, 0},
                                            Vector{14.357224539367046, -2.014269495250773, 0},
                                            Vector{15.26875989292903, -1.6071154615675085, 0},
                                            Vector{16.081236208073463, -1.0269832824330973, 0},
                                            Vector{16.762262618032818, -0.2970009971166694, 0},
                                            Vector{17.284688748945122, 0.5537293042585145, 0},
                                            Vector{17.6276871195819, 1.491291689211344, 0},
                                            Vector{17.777583467299806, 2.478308504102941, 0},
                                            Vector{17.728401897740607, 3.4754305032352506, 0},
                                            Vector{17.482103124881213, 4.442905579103292, 0},
                                            Vector{17.048506303554806, 5.342163553476361, 0},
                                            Vector{16.444897570733772, 6.1373538486888375, 0},
                                            Vector{15.695340901798186, 6.796774737038687, 0},
                                            Vector{14.829718755775655, 7.294137188630348, 0},
                                            Vector{13.882540755999456, 7.609612932036834, 0},
                                            Vector{12.89156790032764, 7.73062494490981, 0},
                                            Vector{11.896307149320169, 7.652348860171518, 0},
                                            Vector{10.936436408392781, 7.377905298306002, 0},
                                            Vector{10.050222694938975, 6.918235458068104, 0},
                                            Vector{9.272996553105063, 6.2916649254160415, 0},
                                            Vector{9.0, 6.0, 0}};

    std::vector<Vector> expected_result2 = {Vector{6.123233995736766e-16, 0.0, 0},
                                            Vector{0.9933466539753065, 0.09966711079379209, 0},
                                            Vector{1.9470917115432524, 0.3946950299855745, 0},
                                            Vector{2.823212366975177, 0.8733219254516085, 0},
                                            Vector{3.5867804544976143, 1.5164664532641732, 0},
                                            Vector{4.207354924039483, 2.2984884706593016, 0},
                                            Vector{4.660195429836132, 3.188211227616632, 0},
                                            Vector{4.9272486499423005, 4.1501642854987955, 0},
                                            Vector{4.997868015207525, 5.145997611506444, 0},
                                            Vector{4.869238154390976, 6.136010473465436, 0},
                                            Vector{4.546487134128409, 7.080734182735712, 0},
                                            Vector{4.0424820190979505, 7.942505586276729, 0},
                                            Vector{3.453414225327183, 8.749607451779864, 0},
                                            Vector{3.0208190902513206, 9.649347723773186, 0},
                                            Vector{2.7755978543530304, 10.617096479082347, 0},
                                            Vector{2.727526714467218, 11.61427262874097, 0},
                                            Vector{2.878522115243877, 12.601121906433475, 0},
                                            Vector{3.222564346547582, 13.538301745641235, 0},
                                            Vector{3.7459375303042997, 14.388449743617045, 0},
                                            Vector{4.427776429277235, 15.11767318247875, 0},
                                            Vector{5.240898278231409, 15.69690022491657, 0},
                                            Vector{6.1528864750033945, 16.10303891660514, 0},
                                            Vector{7.127382928133069, 16.31989778955159, 0},
                                            Vector{8.125537539235536, 16.338831364829296, 0},
                                            Vector{9.107557033825168, 16.15908482054028, 0},
                                            Vector{10.03429139359439, 15.787824084182331, 0},
                                            Vector{10.868794644098593, 15.239850149733034, 0},
                                            Vector{11.57779777416746, 14.537009008727138, 0},
                                            Vector{12.133035066393902, 13.707320719513074, 0},
                                            Vector{12.512370962089452, 12.783862335949, 0},
                                            Vector{12.700682536156371, 11.803449229646171, 0},
                                            Vector{12.690462400388142, 10.805167377280418, 0},
                                            Vector{12.48211799934634, 9.828815126045004, 0},
                                            Vector{12.08395536683173, 8.913316559139, 0},
                                            Vector{11.511847990527825, 8.095169715402147, 0},
                                            Vector{10.788603986138625, 7.4069915276825675, 0},
                                            Vector{9.955581925670863, 6.854885170996804, 0},
                                            Vector{9.192828057382332, 6.210775208064388, 0},
                                            Vector{9.0, 6.0, 0}};

    EXPECT_EQ(result1.size(), expected_result1.size());
    EXPECT_EQ(result2.size(), expected_result2.size());

    // could throw an indexOutOfBounds error
    for (int i = 0; i < result1.size(); i++) {
        EXPECT_NEAR(result1[i].x, expected_result1[i].x, 0.01);
        EXPECT_NEAR(result1[i].y, expected_result1[i].y, 0.01);
    }

    // could throw an indexOutOfBounds error
    for (int i = 0; i < result2.size(); i++) {
        EXPECT_NEAR(result2[i].x, expected_result2[i].x, 0.01);
        EXPECT_NEAR(result2[i].y, expected_result2[i].y, 0.01);
    }
}

/*
 *   tests Dubins::lsl()
 */
TEST(DubinsTest, LSL) {
    Dubins dubins1(5, 10);

    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{73, 41, 0}, 4.00};
    RRTPoint plus_x100{Vector{100, 0, 0}, 0};
    RRTPoint arbitrary_position2{Vector{5, 100, 0}, M_PI / 2};

    RRTOption result1 =
        dubins1.lsl(origin_x, arbitrary_position1, dubins1.findCenter(origin_x, 'L'),
                    dubins1.findCenter(arbitrary_position1, 'L'));
    RRTOption expected_result1{103.46948015930067,
                               DubinsPath{0.40295754, 3.5970424510, 83.46948015930067}, true};

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = dubins1.lsl(origin_x, plus_x100, dubins1.findCenter(origin_x, 'L'),
                                    dubins1.findCenter(plus_x100, 'L'));
    RRTOption expected_result2{100, DubinsPath{0, 0, 100}, true};

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 =
        dubins1.lsl(origin_x, arbitrary_position2, dubins1.findCenter(origin_x, 'L'),
                    dubins1.findCenter(arbitrary_position2, 'L'));
    RRTOption expected_result3{102.85398163397448, DubinsPath{M_PI / 2, 0, 95}, true};

    EXPECT_NEAR(result3.length, expected_result3.length, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_0, expected_result3.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result3.dubins_path.beta_2, expected_result3.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result3.dubins_path.straight_dist, result3.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result3.has_straight, expected_result3.has_straight);
}

/*
 *   tests Dubins::rsr()
 */
TEST(DubinsTest, RSR) {
    Dubins dubins1(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{73, 41, 0}, 4.00};
    RRTPoint plus_x100{Vector{100, 0, 0}, 0};
    RRTPoint arbitrary_position2{Vector{5, -100, 0}, -M_PI / 2};

    RRTOption result1 =
        dubins1.rsr(origin_x, arbitrary_position1, dubins1.findCenter(origin_x, 'R'),
                    dubins1.findCenter(arbitrary_position1, 'R'));
    RRTOption expected_result1(
        127.792, DubinsPath(-5.664581035483313, -2.9017895788758596, 84.96005087111514), true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = dubins1.rsr(origin_x, plus_x100, dubins1.findCenter(origin_x, 'R'),
                                    dubins1.findCenter(plus_x100, 'R'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 =
        dubins1.rsr(origin_x, arbitrary_position2, dubins1.findCenter(origin_x, 'R'),
                    dubins1.findCenter(arbitrary_position2, 'R'));
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
TEST(DubinsTest, RSL) {
    Dubins dubins1(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{73, 41, 0}, 4.00};
    RRTPoint plus_x100{Vector{100, 0, 0}, 0};
    RRTPoint arbitrary_position2{Vector{10, -100, 0}, 0};

    RRTOption result1 =
        dubins1.rsl(origin_x, arbitrary_position1, dubins1.findCenter(origin_x, 'R'),
                    dubins1.findCenter(arbitrary_position1, 'L'));
    RRTOption expected_result1(
        134.78090998278276, DubinsPath(-5.8893974274779834, 3.606212120298397, 87.30286224390085),
        true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = dubins1.rsl(origin_x, plus_x100, dubins1.findCenter(origin_x, 'R'),
                                    dubins1.findCenter(plus_x100, 'L'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 =
        dubins1.rsl(origin_x, arbitrary_position2, dubins1.findCenter(origin_x, 'R'),
                    dubins1.findCenter(arbitrary_position2, 'L'));
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
TEST(DubinsTest, LSR) {
    Dubins dubins1(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{73, 41, 0}, 4.00};
    RRTPoint plus_x100{Vector{100, 0, 0}, 0};
    RRTPoint arbitrary_position2{Vector{10, 100, 0}, 0};

    RRTOption result1 =
        dubins1.lsr(origin_x, arbitrary_position1, dubins1.findCenter(origin_x, 'L'),
                    dubins1.findCenter(arbitrary_position1, 'R'));
    RRTOption expected_result1(
        96.78474229907584, DubinsPath(0.6420440973470476, -2.925229404526634, 78.94837478970744),
        true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = dubins1.lsr(origin_x, plus_x100, dubins1.findCenter(origin_x, 'L'),
                                    dubins1.findCenter(plus_x100, 'R'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 =
        dubins1.lsr(origin_x, arbitrary_position2, dubins1.findCenter(origin_x, 'L'),
                    dubins1.findCenter(arbitrary_position2, 'R'));
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
TEST(DubinsTest, LRL) {
    Dubins dubins1(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    RRTOption result1 =
        dubins1.lrl(origin_x, arbitrary_position1, dubins1.findCenter(origin_x, 'L'),
                    dubins1.findCenter(arbitrary_position1, 'L'));
    RRTOption expected_result1(37.28571149387029,
                               DubinsPath(2.25948315258286, 0.3274953432143759, 4.870163802976823),
                               false);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);
}

/*
 *   tests Dubins::rlr()
 */
TEST(DubinsTest, RLR) {
    Dubins dubins1(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    RRTOption result1 =
        dubins1.rlr(origin_x, arbitrary_position1, dubins1.findCenter(origin_x, 'R'),
                    dubins1.findCenter(arbitrary_position1, 'R'));
    RRTOption expected_result1(
        56.99424154724155, DubinsPath(-1.0585943958426456, -5.782422412471302, 4.557831501134362),
        false);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);
}

/*
 *   tests Dubins::allOptions()
 */
TEST(DubinsTest, AllOptions) {
    Dubins dubins1(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};
    RRTPoint arbitrary_position2{Vector{3, -1, 0}, 2.36};

    std::vector<RRTOption> result1 = dubins1.allOptions(origin_x, arbitrary_position1);
    std::vector<RRTOption> expected_result1 = {
        RRTOption(64.39960045236231,
                  DubinsPath(6.107586558274035, 4.175598748905551, 12.983673916464376), true),
        RRTOption(58.0235802190719,
                  DubinsPath(-5.062863952455051, -3.5035066619041215, 15.191727147276039), true),
        RRTOption(32.99374082753003,
                  DubinsPath(-0.18936765807467593, 4.189367658074676, 11.100064246783269), true),
        RRTOption(std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), true),
        RRTOption(56.99424154724155,
                  DubinsPath(-1.0585943958426456, -5.782422412471302, 4.557831501134362), false),
        RRTOption(37.28571149387029,
                  DubinsPath(2.25948315258286, 0.3274953432143759, -4.870163802976823), false),
    };

    std::vector<RRTOption> result2 = dubins1.allOptions(arbitrary_position1, arbitrary_position2);
    std::vector<RRTOption> expected_result2 = {
        RRTOption(69.79960318782443,
                  DubinsPath(5.92544955425334, 5.000921060105833, 15.167750116028579), true),
        RRTOption(46.460939412666036,
                  DubinsPath(-5.378813639968769, -2.544371667210817, 6.84501287676811), true),
        RRTOption(38.47693182697205,
                  DubinsPath(-0.4132680790016785, 5.056453386181265, 11.128324501057335), true),
        RRTOption(std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), true),
        RRTOption(64.04562997430888,
                  DubinsPath(-1.8879098315046257, -5.336653165926261, 5.584562997430888), false),
        RRTOption(37.41514012689866,
                  DubinsPath(1.9230212534186863, 0.9984927592711799, -4.561514012689866), false)};

    for (int i = 0; i < result1.size(); i++) {
        // if the path is impossible or provablly non-competitive (length == inf),
        // then checking inf is unique, and nothing else needs to be checked (should be garbage
        // data)
        if (std::isinf(result1[i].length) || std::isinf(expected_result1[i].length)) {
            EXPECT_EQ(std::isinf(result1[i].length), std::isinf(expected_result1[i].length));
        } else {
            EXPECT_NEAR(result1[i].length, expected_result1[i].length, 0.01);
            EXPECT_NEAR(result1[i].dubins_path.beta_0, expected_result1[i].dubins_path.beta_0,
                        0.01);
            EXPECT_NEAR(result1[i].dubins_path.beta_2, expected_result1[i].dubins_path.beta_2,
                        0.01);
            EXPECT_NEAR(result1[i].dubins_path.straight_dist,
                        expected_result1[i].dubins_path.straight_dist, 0.01);
        }

        EXPECT_EQ(result1[i].has_straight, expected_result1[i].has_straight);
    }

    for (int i = 0; i < result2.size(); i++) {
        // if the path is impossible or provablly non-competitive (length == inf),
        // then checking inf is unique, and nothing else needs to be checked (should be garbage
        // data)
        if (std::isinf(result2[i].length) || std::isinf(expected_result2[i].length)) {
            EXPECT_EQ(std::isinf(result2[i].length), std::isinf(expected_result2[i].length));
        } else {
            EXPECT_NEAR(result2[i].length, expected_result2[i].length, 0.01);
            EXPECT_NEAR(result2[i].dubins_path.beta_0, expected_result2[i].dubins_path.beta_0,
                        0.01);
            EXPECT_NEAR(result2[i].dubins_path.beta_2, expected_result2[i].dubins_path.beta_2,
                        0.01);
            EXPECT_NEAR(result2[i].dubins_path.straight_dist,
                        expected_result2[i].dubins_path.straight_dist, 0.01);
        }

        EXPECT_EQ(result2[i].has_straight, expected_result2[i].has_straight);
    }
}

/*
 *   tests Dubins::dubinsPath()
 */
TEST(DubinsTest, DubinsPath) {
    Dubins dubins1{5, 1};
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    std::vector<Vector> result1 = dubins1.dubinsPath(origin_x, arbitrary_position1);
    std::vector<Vector> expected_result1 = {Vector{6.123233995736766e-16, 0.0, 0},
                                            Vector{0.993400836368525, -0.09938973742323216, 0},
                                            Vector{1.975524298544812, -0.2876276322341959, 0},
                                            Vector{2.957647760721099, -0.4758655270451597, 0},
                                            Vector{3.939771222897386, -0.6641034218561235, 0},
                                            Vector{4.9218946850736724, -0.8523413166670872, 0},
                                            Vector{5.904018147249959, -1.040579211478051, 0},
                                            Vector{6.886141609426246, -1.2288171062890147, 0},
                                            Vector{7.868265071602533, -1.4170550010999785, 0},
                                            Vector{8.850388533778819, -1.6052928959109423, 0},
                                            Vector{9.832511995955107, -1.793530790721906, 0},
                                            Vector{10.814635458131391, -1.9817686855328698, 0},
                                            Vector{11.79675892030768, -2.1700065803438333, 0},
                                            Vector{12.79027164737483, -2.26821418659457, 0},
                                            Vector{13.783492756324007, -2.1673036486132564, 0},
                                            Vector{14.736867740560596, -1.8710820299393291, 0},
                                            Vector{15.61238854801182, -1.3913587517279438, 0},
                                            Vector{16.375150926950273, -0.7472588672270439, 0},
                                            Vector{16.994745948363555, 0.035539393743043934, 0},
                                            Vector{17.446472313993077, 0.9258283347801974, 0},
                                            Vector{17.712321119146168, 1.8881149452744896, 0},
                                            Vector{17.781693810895792, 2.8840358947365097, 0},
                                            Vector{17.65182471894008, 3.873886957721502, 0},
                                            Vector{17.32789131414969, 4.818205896003914, 0},
                                            Vector{16.822807799142637, 5.67934569348516, 0},
                                            Vector{16.15671025977906, 6.422975423923436, 0},
                                            Vector{15.356153902961042, 7.019448916625484, 0},
                                            Vector{14.453054384333244, 7.444986655718225, 0},
                                            Vector{13.48341543180803, 7.68262379440602, 0},
                                            Vector{12.48589349054992, 7.722886489876743, 0},
                                            Vector{11.500256612494358, 7.56416959551826, 0},
                                            Vector{10.565799029612386, 7.212800653048518, 0},
                                            Vector{9.719774616882088, 6.682787633394971, 0},
                                            Vector{9.0, 6.0, 0}};

    EXPECT_EQ(result1.size(), expected_result1.size());

    for (int i = 0; i < result1.size(); i++) {
        EXPECT_NEAR(result1[i].x, expected_result1[i].x, 0.01);
        EXPECT_NEAR(result1[i].y, expected_result1[i].y, 0.01);
    }
}