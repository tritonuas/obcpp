#include "pathing/dubins.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "utilities/datatypes.hpp"

typedef XYZCoord Vector;

static inline void setDubins(double r, double sep) {
    Dubins::_radius = r;
    Dubins::_point_separation = sep;
}

namespace {

// headings are only meaningful mod 2pi, so compare them on the circle
void expectHeadingNear(double actual, double expected, double tolerance) {
    const double difference = std::abs(mod(actual - expected + M_PI, TWO_PI) - M_PI);
    EXPECT_LT(difference, tolerance) << "heading " << actual << " is not " << expected;
}

void expectPointNear(const Vector& actual, const Vector& expected, double tolerance) {
    EXPECT_NEAR(actual.x, expected.x, tolerance);
    EXPECT_NEAR(actual.y, expected.y, tolerance);
}

/*
 *  The geometry every [LSL, LSR, RSR, RSL] point list has to satisfy:
 *      - it runs from the start vector to the end vector
 *      - every point sits on one of the two turning circles (the straightaway
 *        runs between the tangent points, which are on the circles themselves)
 *      - the turns are sampled at _point_separation, and the straightaway is
 *        described by its two endpoints alone, so that ardupilot accelerates
 *        through it instead of slowing down for every point along the way
 */
void expectStraightPathPoints(const RRTPoint& start, const RRTPoint& end, const DubinsPath& path,
                              const std::vector<Vector>& points) {
    const double radius = Dubins::_radius;
    const Vector center_0 = Dubins::findCenter(start, (path.beta_0 > 0) ? 'L' : 'R');
    const Vector center_2 = Dubins::findCenter(end, (path.beta_2 > 0) ? 'L' : 'R');

    ASSERT_GE(points.size(), 2);
    expectPointNear(points.front(), start.coord, 1e-6);
    expectPointNear(points.back(), end.coord, 1e-6);

    for (const Vector& point : points) {
        const double off_first_turn = std::abs(point.distanceTo(center_0) - radius);
        const double off_last_turn = std::abs(point.distanceTo(center_2) - radius);
        EXPECT_LT(std::min(off_first_turn, off_last_turn), 1e-6)
            << "(" << point.x << ", " << point.y << ") is on neither turning circle";
    }

    int straightaways = 0;
    double flown = 0;
    for (std::size_t i = 1; i < points.size(); i++) {
        const double step = points[i].distanceTo(points[i - 1]);
        flown += step;

        if (step > Dubins::_point_separation + 1e-6) {
            straightaways++;
            EXPECT_NEAR(step, path.straight_dist, 1e-6);
        }
    }
    EXPECT_EQ(straightaways, (path.straight_dist > Dubins::_point_separation) ? 1 : 0);

    // the polyline cuts the corner off of every arc, so it comes out a little
    // shorter than the path it approximates
    const double length =
        radius * (std::abs(path.beta_0) + std::abs(path.beta_2)) + path.straight_dist;
    EXPECT_LT(flown, length + 1e-6);
    EXPECT_GT(flown, length * 0.99);
}

}  // namespace

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
    setDubins(5, 10);

    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    Vector result1 = Dubins::findCenter(origin_x, 'L');
    Vector expected_result1{0.0, 5.0, 0};
    Vector result2 = Dubins::findCenter(origin_x, 'R');
    Vector expected_result2{0.0, -5.0, 0};

    // points towards e2
    RRTPoint origin_y{Vector{0, 0, 0}, M_PI / 2};
    Vector result3 = Dubins::findCenter(origin_y, 'L');
    Vector expected_result3{-5.0, 0.0, 0};
    Vector result4 = Dubins::findCenter(origin_y, 'R');
    Vector expected_result4{5.0, 0.0, 0};

    RRTPoint arbitrary{Vector{12, 156, 100}, 1.3};
    Vector result5 = Dubins::findCenter(arbitrary, 'L');
    // [-4.817, 1.341] ==> magnitude 5 * e1 vector rotated 2.87 [1.3 + pi/2] raidans
    Vector expected_result5{12 - 4.817, 156 + 1.341, 0};
    Vector result6 = Dubins::findCenter(arbitrary, 'R');
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
    setDubins(5, 10);

    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint origin_y{Vector{0, 0, 0}, M_PI / 2};
    RRTPoint arbitrary_position1{Vector{73, 41, 0}, 4.00};

    // plane is facing x+, turning left/ccw with a turning radius of 5,
    // this should be the point where it turns 90deg (1/4 of the circle)
    Vector result1 =
        Dubins::circleArc(origin_x, 1, Dubins::findCenter(origin_x, 'L'), M_PI / 2 * 5);
    Vector expected_result1{5.0, 5.0, 0};

    // plance facing x+, turning right/cw with a turning radius of 5
    // turning 2.97 rad
    Vector result2 = Dubins::circleArc(origin_x, -1, Dubins::findCenter(origin_x, 'R'), 2.97 * 5);
    Vector expected_result2{0.850, -9.927, 0};

    Vector result3 = Dubins::circleArc(arbitrary_position1, 1,
                                       Dubins::findCenter(arbitrary_position1, 'L'), 5.12 * 5);
    Vector expected_result3{78.28441936, 42.50134993, 0};

    Vector result4 = Dubins::circleArc(origin_y, 1, Dubins::findCenter(origin_y, 'L'), M_PI * 5);
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
 */
TEST(DubinsTest, GenPointsStraight) {
    setDubins(5, 1);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    // lsl  origin_x ==> arbitrary_position
    DubinsPath lsl{6.107586558274035, 4.175598748905551, 12.983673916464376};
    expectStraightPathPoints(origin_x, arbitrary_position1, lsl,
                             Dubins::generatePointsStraight(origin_x, arbitrary_position1, lsl));

    // rsr  origin_x ==> arbitrary_position, the same two vectors turning the other way
    DubinsPath rsr{-5.062863952455051, -3.5035066619041215, 15.191727147276039};
    expectStraightPathPoints(origin_x, arbitrary_position1, rsr,
                             Dubins::generatePointsStraight(origin_x, arbitrary_position1, rsr));

    // rsl  origin_x ==> arbitrary_position, one turn each way
    DubinsPath rsl{-0.18936765807467593, 4.189367658074676, 11.100064246783269};
    expectStraightPathPoints(origin_x, arbitrary_position1, rsl,
                             Dubins::generatePointsStraight(origin_x, arbitrary_position1, rsl));

    // a path that never turns is nothing but the straightaway, so it is described
    // by its two endpoints
    RRTPoint straight_end{Vector{20, 0, 0}, 0};
    std::vector<Vector> straight =
        Dubins::generatePointsStraight(origin_x, straight_end, DubinsPath(0, 0, 20));

    ASSERT_EQ(straight.size(), 2);
    expectPointNear(straight[0], origin_x.coord, 1e-6);
    expectPointNear(straight[1], straight_end.coord, 1e-6);
}

/*
 *   tests Dubins::generatePointsCurve()
 */
TEST(DubinsTest, GenPointsCurve) {
    setDubins(5, 1);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    // lrl  origin_x ==> arbitrary_position
    DubinsPath path{2.25948315258286, 0.3274953432143759, 4.870163802976823};

    std::vector<Vector> result1 = Dubins::generatePointsCurve(origin_x, arbitrary_position1, path);
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
 *
 *   generatePoints only picks which of the two generators to hand the path to
 */
TEST(DubinsTest, GenPoints) {
    setDubins(5, 1);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    // straight path
    RRTOption lsl{64.39960045236231,
                  DubinsPath{6.107586558274035, 4.175598748905551, 12.983673916464376}, true};
    // curve only path
    RRTOption lrl{37.28571149387029,
                  DubinsPath{2.25948315258286, 0.3274953432143759, -4.870163802976823}, false};

    const std::vector<Vector> straight = Dubins::generatePoints(
        origin_x, arbitrary_position1, lsl.dubins_path, lsl.has_straight);
    const std::vector<Vector> expected_straight =
        Dubins::generatePointsStraight(origin_x, arbitrary_position1, lsl.dubins_path);

    ASSERT_EQ(straight.size(), expected_straight.size());
    for (std::size_t i = 0; i < straight.size(); i++) {
        expectPointNear(straight[i], expected_straight[i], 1e-9);
    }
    expectStraightPathPoints(origin_x, arbitrary_position1, lsl.dubins_path, straight);

    const std::vector<Vector> curve = Dubins::generatePoints(
        origin_x, arbitrary_position1, lrl.dubins_path, lrl.has_straight);
    // generatePointsCurve reads the middle turn out of straight_dist, and wants it
    // as a magnitude
    const std::vector<Vector> expected_curve = Dubins::generatePointsCurve(
        origin_x, arbitrary_position1, lrl.dubins_path);

    ASSERT_EQ(curve.size(), expected_curve.size());
    for (std::size_t i = 0; i < curve.size(); i++) {
        expectPointNear(curve[i], expected_curve[i], 1e-9);
    }

    // a path that is all turns is sampled the whole way -- there is no straightaway
    // to skip over
    expectPointNear(curve.front(), origin_x.coord, 1e-6);
    expectPointNear(curve.back(), arbitrary_position1.coord, 1e-6);
    for (std::size_t i = 1; i < curve.size(); i++) {
        EXPECT_LT(curve[i].distanceTo(curve[i - 1]), Dubins::_point_separation + 1e-6);
    }
}

/*
 *   tests Dubins::lsl()
 */
TEST(DubinsTest, LSL) {
    setDubins(5, 10);

    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{73, 41, 0}, 4.00};
    RRTPoint plus_x100{Vector{100, 0, 0}, 0};
    RRTPoint arbitrary_position2{Vector{5, 100, 0}, M_PI / 2};

    RRTOption result1 =
        Dubins::lsl(origin_x, arbitrary_position1, Dubins::findCenter(origin_x, 'L'),
                    Dubins::findCenter(arbitrary_position1, 'L'));
    RRTOption expected_result1{103.46948015930067,
                               DubinsPath{0.40295754, 3.5970424510, 83.46948015930067}, true};

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = Dubins::lsl(origin_x, plus_x100, Dubins::findCenter(origin_x, 'L'),
                                    Dubins::findCenter(plus_x100, 'L'));
    RRTOption expected_result2{100, DubinsPath{0, 0, 100}, true};

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 =
        Dubins::lsl(origin_x, arbitrary_position2, Dubins::findCenter(origin_x, 'L'),
                    Dubins::findCenter(arbitrary_position2, 'L'));
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
    setDubins(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{73, 41, 0}, 4.00};
    RRTPoint plus_x100{Vector{100, 0, 0}, 0};
    RRTPoint arbitrary_position2{Vector{5, -100, 0}, -M_PI / 2};

    RRTOption result1 =
        Dubins::rsr(origin_x, arbitrary_position1, Dubins::findCenter(origin_x, 'R'),
                    Dubins::findCenter(arbitrary_position1, 'R'));
    RRTOption expected_result1(
        127.792, DubinsPath(-5.664581035483313, -2.9017895788758596, 84.96005087111514), true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = Dubins::rsr(origin_x, plus_x100, Dubins::findCenter(origin_x, 'R'),
                                    Dubins::findCenter(plus_x100, 'R'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 =
        Dubins::rsr(origin_x, arbitrary_position2, Dubins::findCenter(origin_x, 'R'),
                    Dubins::findCenter(arbitrary_position2, 'R'));
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
    setDubins(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{73, 41, 0}, 4.00};
    RRTPoint plus_x100{Vector{100, 0, 0}, 0};
    RRTPoint arbitrary_position2{Vector{10, -100, 0}, 0};

    RRTOption result1 =
        Dubins::rsl(origin_x, arbitrary_position1, Dubins::findCenter(origin_x, 'R'),
                    Dubins::findCenter(arbitrary_position1, 'L'));
    RRTOption expected_result1(
        134.78090998278276, DubinsPath(-5.8893974274779834, 3.606212120298397, 87.30286224390085),
        true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = Dubins::rsl(origin_x, plus_x100, Dubins::findCenter(origin_x, 'R'),
                                    Dubins::findCenter(plus_x100, 'L'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 =
        Dubins::rsl(origin_x, arbitrary_position2, Dubins::findCenter(origin_x, 'R'),
                    Dubins::findCenter(arbitrary_position2, 'L'));
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
    setDubins(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{73, 41, 0}, 4.00};
    RRTPoint plus_x100{Vector{100, 0, 0}, 0};
    RRTPoint arbitrary_position2{Vector{10, 100, 0}, 0};

    RRTOption result1 =
        Dubins::lsr(origin_x, arbitrary_position1, Dubins::findCenter(origin_x, 'L'),
                    Dubins::findCenter(arbitrary_position1, 'R'));
    RRTOption expected_result1(
        96.78474229907584, DubinsPath(0.6420440973470476, -2.925229404526634, 78.94837478970744),
        true);

    EXPECT_NEAR(result1.length, expected_result1.length, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_0, expected_result1.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result1.dubins_path.beta_2, expected_result1.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result1.dubins_path.straight_dist, result1.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result1.has_straight, expected_result1.has_straight);

    RRTOption result2 = Dubins::lsr(origin_x, plus_x100, Dubins::findCenter(origin_x, 'L'),
                                    Dubins::findCenter(plus_x100, 'R'));
    RRTOption expected_result2(100, DubinsPath(0, 0, 100), true);

    EXPECT_NEAR(result2.length, expected_result2.length, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_0, expected_result2.dubins_path.beta_0, 0.01);
    EXPECT_NEAR(result2.dubins_path.beta_2, expected_result2.dubins_path.beta_2, 0.01);
    EXPECT_NEAR(result2.dubins_path.straight_dist, result2.dubins_path.straight_dist, 0.01);
    EXPECT_EQ(result2.has_straight, expected_result2.has_straight);

    RRTOption result3 =
        Dubins::lsr(origin_x, arbitrary_position2, Dubins::findCenter(origin_x, 'L'),
                    Dubins::findCenter(arbitrary_position2, 'R'));
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
    setDubins(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    RRTOption result1 =
        Dubins::lrl(origin_x, arbitrary_position1, Dubins::findCenter(origin_x, 'L'),
                    Dubins::findCenter(arbitrary_position1, 'L'));
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
    setDubins(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    RRTOption result1 =
        Dubins::rlr(origin_x, arbitrary_position1, Dubins::findCenter(origin_x, 'R'),
                    Dubins::findCenter(arbitrary_position1, 'R'));
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
    setDubins(5, 10);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};
    RRTPoint arbitrary_position2{Vector{3, -1, 0}, 2.36};

    std::vector<RRTOption> result1 = Dubins::allOptions(origin_x, arbitrary_position1);
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

    std::vector<RRTOption> result2 = Dubins::allOptions(arbitrary_position1, arbitrary_position2);
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
    setDubins(5, 1);
    // points towards e1
    RRTPoint origin_x{Vector{0, 0, 0}, 0};
    RRTPoint arbitrary_position1{Vector{9, 6, 0}, 4.00};

    // the path flown is the shortest option there is
    const RRTOption best = Dubins::bestOption(origin_x, arbitrary_position1);
    for (const RRTOption& option : Dubins::allOptions(origin_x, arbitrary_position1)) {
        EXPECT_LE(best.length, option.length);
    }

    const std::vector<Vector> result = Dubins::dubinsPath(origin_x, arbitrary_position1);
    const std::vector<Vector> expected = Dubins::generatePoints(
        origin_x, arbitrary_position1, best.dubins_path, best.has_straight);

    ASSERT_EQ(result.size(), expected.size());
    for (std::size_t i = 0; i < result.size(); i++) {
        expectPointNear(result[i], expected[i], 1e-9);
    }

    ASSERT_TRUE(best.has_straight);
    expectStraightPathPoints(origin_x, arbitrary_position1, best.dubins_path, result);
}

/*
 * ============================================================================
 *  Dubins::generatePath
 *
 *  A sequence of dubins paths, each one flown from the vector the previous one
 *  ended on, stitched into a single list of points.
 * ============================================================================
 */

/*
 *   tests Dubins::generatePath() -- nothing to fly
 */
TEST(DubinsTest, GeneratePathWithNoSegments) {
    setDubins(5, 1);

    EXPECT_TRUE(Dubins::generatePath(RRTPoint{Vector{0, 0, 0}, 0}, {}).empty());
}

/*
 *   tests Dubins::generatePath() -- a single segment, which is the points of that
 *   segment without the point the plane is already sitting on
 */
TEST(DubinsTest, GeneratePathWithOneSegment) {
    setDubins(5, 1);

    const RRTPoint start{Vector{0, 0, 0}, 0};
    const RRTPoint end{Vector{9, 6, 0}, 4.00};
    const RRTOption option = Dubins::bestOption(start, end);

    const std::vector<Vector> expected =
        Dubins::generatePoints(start, end, option.dubins_path, option.has_straight);
    const std::vector<Vector> path = Dubins::generatePath(start, {PathSegment(end, option)});

    ASSERT_EQ(path.size(), expected.size() - 1);
    for (std::size_t i = 0; i < path.size(); i++) {
        expectPointNear(path[i], expected[i + 1], 1e-6);
    }

    // the plane's own position is not repeated, and the path lands on the end
    EXPECT_GT(path.front().distanceTo(start.coord), 0);
    expectPointNear(path.back(), end.coord, 1e-6);
}

/*
 *   tests Dubins::generatePath() -- segments are flown back to back, each one
 *   starting where the last one ended
 */
TEST(DubinsTest, GeneratePathChainsSegments) {
    setDubins(5, 1);

    const RRTPoint start{Vector{0, 0, 0}, 0};
    const std::vector<RRTPoint> waypoints = {
        RRTPoint{Vector{30, 10, 0}, HALF_PI},
        RRTPoint{Vector{10, 50, 0}, M_PI},
        RRTPoint{Vector{-30, 20, 0}, 3 * HALF_PI},
    };

    std::vector<PathSegment> segments;
    RRTPoint current = start;
    for (const RRTPoint& waypoint : waypoints) {
        segments.emplace_back(waypoint, Dubins::bestOption(current, waypoint));
        current = waypoint;
    }

    const std::vector<Vector> path = Dubins::generatePath(start, segments);
    ASSERT_FALSE(path.empty());

    // the path visits every waypoint, in order, and ends on the last one
    std::size_t index = 0;
    for (const RRTPoint& waypoint : waypoints) {
        bool found = false;
        for (; index < path.size(); index++) {
            if (path[index].distanceTo(waypoint.coord) < 1e-6) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "path never reached (" << waypoint.coord.x << ", "
                           << waypoint.coord.y << ")";
    }
    expectPointNear(path.back(), waypoints.back().coord, 1e-6);

    // the joints are not duplicated -- no two points in a row are identical
    for (std::size_t i = 1; i < path.size(); i++) {
        EXPECT_GT(path[i].distanceTo(path[i - 1]), 0)
            << "duplicate point at index " << i;
    }

    // the segments concatenate: dropping the first point of each one accounts for
    // every point in the path
    std::size_t expected_size = 0;
    current = start;
    for (const PathSegment& segment : segments) {
        expected_size += Dubins::generatePoints(current, segment.end, segment.option.dubins_path,
                                                segment.option.has_straight)
                             .size() -
                         1;
        current = segment.end;
    }
    EXPECT_EQ(path.size(), expected_size);
}
