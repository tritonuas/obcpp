#include "pathing/environment.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <random>
#include <vector>

#include "pathing/dubins.hpp"
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
    Environment::init(test, {}, {});

    EXPECT_EQ(true, Environment::isPointInPolygon(test, XYZCoord{0.5, 0.5, 0}));
    EXPECT_EQ(true, Environment::isPointInPolygon(test, XYZCoord{0.5, 0.5, 99999999}));

    EXPECT_EQ(false, Environment::isPointInPolygon(test, XYZCoord{1, 0.5, 0}));   // edge is outside
    EXPECT_EQ(false, Environment::isPointInPolygon(test, XYZCoord{2, 0.5, 0}));   // right
    EXPECT_EQ(false, Environment::isPointInPolygon(test, XYZCoord{0.5, 2, 0}));   // top
    EXPECT_EQ(false, Environment::isPointInPolygon(test, XYZCoord{-1, 0.5, 0}));  // left
    EXPECT_EQ(false, Environment::isPointInPolygon(test, XYZCoord{0.5, -1, 0}));  // down

    Polygon no_point = {};
    Environment::init(no_point, {}, {});

    EXPECT_EQ(false, Environment::isPointInPolygon(no_point, XYZCoord{1, 1, 1}));
    EXPECT_EQ(false, Environment::isPointInPolygon(no_point, XYZCoord{1, 0, 1}));
    EXPECT_EQ(false, Environment::isPointInPolygon(no_point, XYZCoord{0, 1, 1}));

    Polygon point;
    point.emplace_back(XYZCoord{1, 1, 1});
    Environment::init(point, {}, {});

    EXPECT_EQ(false, Environment::isPointInPolygon(point, XYZCoord{1, 1, 1}));
    EXPECT_EQ(false, Environment::isPointInPolygon(point, XYZCoord{1, 0, 1}));
    EXPECT_EQ(false, Environment::isPointInPolygon(point, XYZCoord{0, 1, 1}));

    // tests close to diagonals
    Polygon quadrilateral;
    quadrilateral.emplace_back(XYZCoord{0, 0, 0});
    quadrilateral.emplace_back(XYZCoord{2, 1, 0});
    quadrilateral.emplace_back(XYZCoord{4, 4, 0});
    quadrilateral.emplace_back(XYZCoord{1, 2, 0});
    Environment::init(quadrilateral, {}, {});

    EXPECT_EQ(true, Environment::isPointInPolygon(quadrilateral, XYZCoord{1.5, 1.00, 0}));
    EXPECT_EQ(true, Environment::isPointInPolygon(quadrilateral, XYZCoord{0.5, 0.90, 0}));
    EXPECT_EQ(true, Environment::isPointInPolygon(quadrilateral, XYZCoord{2.5, 2.00, 0}));
    EXPECT_EQ(true, Environment::isPointInPolygon(quadrilateral, XYZCoord{1.5, 2.25, 0}));

    EXPECT_EQ(false, Environment::isPointInPolygon(quadrilateral, XYZCoord{1.5, 0.75, 0}));
    EXPECT_EQ(false, Environment::isPointInPolygon(quadrilateral, XYZCoord{0.5, 1.10, 0}));
    EXPECT_EQ(false, Environment::isPointInPolygon(quadrilateral, XYZCoord{2.5, 1.30, 0}));
    EXPECT_EQ(false, Environment::isPointInPolygon(quadrilateral, XYZCoord{1.5, 2.50, 0}));
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
    Environment::init(small_square, {}, {}, obstacles);

    EXPECT_EQ(true, Environment::isPointInBounds(XYZCoord{0.5, 0.5, 0}));
    EXPECT_EQ(true, Environment::isPointInBounds(XYZCoord{0.5, 0.5, 99999999}));

    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{1, 0.5, 0}));   // edge is outside
    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{2, 0.5, 0}));   // right
    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{0.5, 2, 0}));   // top
    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{-1, 0.5, 0}));  // left
    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{0.5, -1, 0}));  // down

    Polygon no_point_polygon;
    Environment::init(no_point_polygon, {}, {}, obstacles);

    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{1, 1, 1}));
    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{1, 0, 1}));
    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{0, 1, 1}));

    Polygon point_polygon;
    point_polygon.emplace_back(XYZCoord{1, 1, 1});

    Environment::init(point_polygon, {}, {}, obstacles);

    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{1, 1, 1}));
    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{1, 0, 1}));
    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{0, 1, 1}));

    // tests close to diagonals
    Polygon quadrateral_polygon;
    quadrateral_polygon.emplace_back(XYZCoord{0, 0, 0});
    quadrateral_polygon.emplace_back(XYZCoord{2, 1, 0});
    quadrateral_polygon.emplace_back(XYZCoord{4, 4, 0});
    quadrateral_polygon.emplace_back(XYZCoord{1, 2, 0});

    Environment::init(quadrateral_polygon, {}, {}, obstacles);

    EXPECT_EQ(true, Environment::isPointInBounds(XYZCoord{1.5, 1.00, 0}));
    EXPECT_EQ(true, Environment::isPointInBounds(XYZCoord{0.5, 0.90, 0}));
    EXPECT_EQ(true, Environment::isPointInBounds(XYZCoord{2.5, 2.00, 0}));
    EXPECT_EQ(true, Environment::isPointInBounds(XYZCoord{1.5, 2.25, 0}));

    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{1.5, 0.75, 0}));
    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{0.5, 1.10, 0}));
    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{2.5, 1.30, 0}));
    EXPECT_EQ(false, Environment::isPointInBounds(XYZCoord{1.5, 2.50, 0}));
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

    Environment::init(small_square, {}, {}, obstacles);

    // TODO --> REALLY SHIT TEST
    std::vector<XYZCoord> path_in_bounds{XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999},
                                         XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999}};

    std::vector<XYZCoord> path_out_of_bounds{XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999},
                                             XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999},
                                             XYZCoord{1, 0.5, 0},  // edge is outside
                                             XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999},
                                             XYZCoord{0.5, 0.5, 0}, XYZCoord{0.5, 0.5, 99999999}};

    EXPECT_EQ(true, Environment::isPathInBounds(path_in_bounds));
    EXPECT_EQ(false, Environment::isPathInBounds(path_out_of_bounds));
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

    Environment::init(square, {}, {}, obstacles);

    EXPECT_FALSE(Environment::isPointInBounds(XYZCoord{15, 15, 0}));
    EXPECT_FALSE(Environment::isPointInBounds(XYZCoord{10, 10, 0}));
    EXPECT_FALSE(Environment::isPointInBounds(XYZCoord{15, 10, 0}));

    // for sanity
    EXPECT_TRUE(Environment::isPointInBounds(XYZCoord{1, 1, 0}));
}

/*
 *   tests Environment::intersect()
 */
TEST(EnvironmentTest, IntersectTest) {
    Environment::init({}, {}, {});

    // test intersect
    std::vector<XYZCoord> path1 = {XYZCoord{0, 0, 0}, XYZCoord{100, 100, 0}};
    std::vector<XYZCoord> path1_5 = {XYZCoord{0, 0, 0}, XYZCoord{-1, -1, 0}};
    std::vector<XYZCoord> path2 = {XYZCoord{-1, -1, 0}, XYZCoord{5, 5, 0}};
    std::vector<XYZCoord> path3 = {XYZCoord{0, 0, 0}, XYZCoord{15, 15, 0}};
    std::vector<XYZCoord> path4 = {XYZCoord{2, 1, 0}, XYZCoord{1, 2, 0}};

    EXPECT_TRUE(Environment::intersect(path1[0], path1[1], path2[0], path2[1]));
    EXPECT_TRUE(Environment::intersect(path1[0], path1[1], path1_5[0], path1_5[1]));

    EXPECT_TRUE(Environment::intersect(path1[0], path1[1], path3[0], path3[1]));
    EXPECT_TRUE(Environment::intersect(path2[0], path2[1], path3[0], path3[1]));

    EXPECT_TRUE(Environment::intersect(path1[0], path1[1], path4[0], path4[1]));
    EXPECT_FALSE(Environment::intersect(path1_5[0], path1_5[1], path4[0], path4[1]));
}

/*
 *   tests Environment::verticalRayIntersectsEdge()
 */
TEST(EnvironmentTest, VerticalRayIntersectsEdge) {
    Polygon airdrop_zone = {
        {XYZCoord(0, 0, 0), XYZCoord(100, 0, 0), XYZCoord(100, 100, 0), XYZCoord(50, 100, 0)}};

    Environment::init({}, airdrop_zone, {});

    std::pair<XYZCoord, XYZCoord> edge1 = {XYZCoord(75, 9999, 0), XYZCoord(75, -9999, 0)};
    XYZCoord intersect1(0, 0, 0);
    XYZCoord expect1(75, 0, 0);
    XYZCoord expect2(75, 100, 0);

    EXPECT_TRUE(Environment::verticalRayIntersectsEdge(airdrop_zone[0], airdrop_zone[1], edge1.first,
                                               edge1.second, intersect1));
    EXPECT_EQ(intersect1, expect1);
    EXPECT_FALSE(Environment::verticalRayIntersectsEdge(airdrop_zone[1], airdrop_zone[2], edge1.first,
                                                edge1.second, intersect1));
    EXPECT_TRUE(Environment::verticalRayIntersectsEdge(airdrop_zone[2], airdrop_zone[3], edge1.first,
                                               edge1.second, intersect1));
    EXPECT_EQ(intersect1, expect2);
    EXPECT_FALSE(Environment::verticalRayIntersectsEdge(airdrop_zone[3], airdrop_zone[0], edge1.first,
                                                edge1.second, intersect1));

    std::pair<XYZCoord, XYZCoord> edge2 = {XYZCoord(25, 9999, 0), XYZCoord(25, -9999, 0)};
    XYZCoord intersect2(0, 0, 0);
    XYZCoord expect3(25, 0, 0);
    XYZCoord expect4(25, 50, 0);
    EXPECT_TRUE(Environment::verticalRayIntersectsEdge(airdrop_zone[0], airdrop_zone[1], edge2.first,
                                               edge2.second, intersect2));
    EXPECT_EQ(intersect2, expect3);
    EXPECT_FALSE(Environment::verticalRayIntersectsEdge(airdrop_zone[1], airdrop_zone[2], edge2.first,
                                                edge2.second, intersect2));
    EXPECT_FALSE(Environment::verticalRayIntersectsEdge(airdrop_zone[2], airdrop_zone[3], edge2.first,
                                                edge2.second, intersect2));
    EXPECT_TRUE(Environment::verticalRayIntersectsEdge(airdrop_zone[3], airdrop_zone[0], edge2.first,
                                               edge2.second, intersect2));
    EXPECT_EQ(intersect2, expect4);
}

/*
 *   tests Environment::findIntersectionsWithPolygon()
 */
TEST(EnvironmentTest, FindIntersectionsWithPolygon) {
    Polygon mapping_region1 = {
        {XYZCoord(10, 10, 0), XYZCoord(100, 10, 0), XYZCoord(100, 100, 0), XYZCoord(50, 100, 0)}};
    Environment::init({}, {}, mapping_region1);

    XYZCoord start1(0.0, 0.0, 0.0);
    XYZCoord end1(120.0, 50.0, 0.0);

    std::vector<XYZCoord> expect1 = {XYZCoord(24.0, 10.0, 0.0), XYZCoord(100.0, 500.0 / 12, 0)};
    std::vector<XYZCoord> result1 =
        Environment::findIntersectionsWithPolygon(mapping_region1, start1, end1);
    EXPECT_EQ(expect1, result1);

    XYZCoord start2(20.0, 20.0, 0.0);
    XYZCoord end2(120.0, 50.0, 0.0);
    std::vector<XYZCoord> expect2 = {XYZCoord(100.0, 44.0, 0.0)};
    std::vector<XYZCoord> result2 =
        Environment::findIntersectionsWithPolygon(mapping_region1, start2, end2);
    EXPECT_EQ(expect2, result2);

    XYZCoord start3(0.0, 0.0, 0.0);
    XYZCoord end3(50.0, 50.0, 0.0);
    std::vector<XYZCoord> expect3 = {XYZCoord(10.0, 10.0, 0.0)};
    std::vector<XYZCoord> result3 =
        Environment::findIntersectionsWithPolygon(mapping_region1, start3, end3);
    EXPECT_EQ(expect3, result3);

    Polygon mapping_region2 = {{XYZCoord(0.0, 0.0, 0.0), XYZCoord(50.0, 100.0, 0.0),
                                XYZCoord(100.0, 0.0, 0.0), XYZCoord(150.0, 100.0, 0.0),
                                XYZCoord(200.0, 0.0, 0.0)}};
    Environment::init({}, {}, mapping_region2);

    XYZCoord start4(0.0, 50.0, 0.0);
    XYZCoord end4(200.0, 50.0, 0.0);
    std::vector<XYZCoord> expect4 = {XYZCoord(25.0, 50.0, 0.0), XYZCoord(75.0, 50.0, 0.0),
                                     XYZCoord(125.0, 50.0, 0.0), XYZCoord(175.0, 50.0, 0.0)};
    std::vector<XYZCoord> result4 =
        Environment::findIntersectionsWithPolygon(mapping_region2, start4, end4);
    EXPECT_EQ(expect4, result4);

    // test with start point and end point on the edge; they should be included in the intersections
    XYZCoord start5(25.0, 50.0, 0.0);
    XYZCoord end5(175.0, 50.0, 0.0);
    std::vector<XYZCoord> expect5 = {XYZCoord(25.0, 50.0, 0.0), XYZCoord(75.0, 50.0, 0.0),
                                     XYZCoord(125.0, 50.0, 0.0), XYZCoord(175.0, 50.0, 0.0)};
    std::vector<XYZCoord> result5 =
        Environment::findIntersectionsWithPolygon(mapping_region2, start5, end5);
    EXPECT_EQ(expect5, result5);
}
/*
 * ============================================================================
 *  Environment::isDubinsPathInBounds and its arc helpers
 *
 *  The analytic checks are allowed to be CONSERVATIVE (reject a path that is
 *  actually clear, e.g. one that grazes a boundary), but must never be
 *  PERMISSIVE (accept a path that leaves the region or clips an obstacle).
 *  The differential tests below assert that direction specifically.
 * ============================================================================
 */

namespace {

// 100 x 100 field, nothing in it
void initOpenField() {
    Polygon field = {{XYZCoord(0, 0, 0), XYZCoord(100, 0, 0), XYZCoord(100, 100, 0),
                      XYZCoord(0, 100, 0)}};
    Environment::init(field, {}, {}, {});
}

// 100 x 100 field with a 20 x 20 obstacle dead center, spanning [40, 60] on both axes
void initFieldWithObstacle() {
    Polygon field = {{XYZCoord(0, 0, 0), XYZCoord(100, 0, 0), XYZCoord(100, 100, 0),
                      XYZCoord(0, 100, 0)}};
    Polygon obstacle = {{XYZCoord(40, 40, 0), XYZCoord(60, 40, 0), XYZCoord(60, 60, 0),
                         XYZCoord(40, 60, 0)}};
    Environment::init(field, {}, {}, {obstacle});
}

// Reference implementation: walk the arc point by point. Slow and dumb on purpose --
// this is what the analytic check is verified against.
bool arcInBoundsBruteForce(const XYZCoord& center, double radius, double start_angle, double sweep,
                           int steps = 4000) {
    for (int i = 0; i <= steps; i++) {
        const double angle = start_angle + sweep * (static_cast<double>(i) / steps);
        if (!Environment::isPointInBounds(
                XYZCoord{center.x + radius * std::cos(angle),
                         center.y + radius * std::sin(angle), 0})) {
            return false;
        }
    }
    return true;
}

// Reference implementation: generate the whole Dubins curve and check every point.
// This is the check isDubinsPathInBounds replaces.
bool dubinsInBoundsBruteForce(const RRTPoint& start, const RRTPoint& end, const RRTOption& option) {
    if (!std::isfinite(option.length)) {
        return false;
    }

    for (const XYZCoord& point :
         Dubins::generatePoints(start, end, option.dubins_path, option.has_straight)) {
        if (!Environment::isPointInBounds(point)) {
            return false;
        }
    }
    return true;
}

}  // namespace

/*
 *  Environment::doesArcIntersectSegment -- a segment that never reaches the circle
 */
TEST(DubinsBoundsTest, ArcSegmentMissesCircleEntirely) {
    initOpenField();
    const XYZCoord center(0, 0, 0);

    // circle of radius 10, segment sitting out at x = 20
    EXPECT_FALSE(Environment::doesArcIntersectSegment(center, 10, 0, TWO_PI, XYZCoord(20, -20, 0),
                                                      XYZCoord(20, 20, 0)));
    // segment pointing at the circle but stopping short
    EXPECT_FALSE(Environment::doesArcIntersectSegment(center, 10, 0, TWO_PI, XYZCoord(11, 0, 0),
                                                      XYZCoord(50, 0, 0)));
}

/*
 *  Environment::doesArcIntersectSegment -- the angular window is respected
 *
 *  The vertical segment x = 5 cuts the radius-10 circle at +-60 degrees.
 */
TEST(DubinsBoundsTest, ArcSegmentRespectsSweepWindow) {
    initOpenField();
    const XYZCoord center(0, 0, 0);
    const XYZCoord seg_start(5, -20, 0);
    const XYZCoord seg_end(5, 20, 0);

    // 0 -> 90 deg contains the +60 deg hit
    EXPECT_TRUE(
        Environment::doesArcIntersectSegment(center, 10, 0, HALF_PI, seg_start, seg_end));
    // 90 -> 180 deg contains neither hit
    EXPECT_FALSE(
        Environment::doesArcIntersectSegment(center, 10, HALF_PI, HALF_PI, seg_start, seg_end));
    // 180 -> 270 deg contains neither hit
    EXPECT_FALSE(
        Environment::doesArcIntersectSegment(center, 10, M_PI, HALF_PI, seg_start, seg_end));
    // the full circle always contains both
    EXPECT_TRUE(Environment::doesArcIntersectSegment(center, 10, 0, TWO_PI, seg_start, seg_end));
}

/*
 *  Environment::doesArcIntersectSegment -- sweep sign selects the direction travelled
 */
TEST(DubinsBoundsTest, ArcSegmentRespectsSweepDirection) {
    initOpenField();
    const XYZCoord center(0, 0, 0);
    const XYZCoord seg_start(5, -20, 0);
    const XYZCoord seg_end(5, 20, 0);

    // starting at 0 and turning CCW a quarter turn reaches +60 deg, but not -60 deg
    EXPECT_TRUE(Environment::doesArcIntersectSegment(center, 10, 0, HALF_PI, seg_start, seg_end));
    // starting at 0 and turning CW a quarter turn reaches -60 deg, but not +60 deg
    EXPECT_TRUE(Environment::doesArcIntersectSegment(center, 10, 0, -HALF_PI, seg_start, seg_end));

    // a segment that only crosses on the +y side, to tell the two directions apart
    const XYZCoord upper_start(-20, 9, 0);
    const XYZCoord upper_end(20, 9, 0);
    EXPECT_TRUE(
        Environment::doesArcIntersectSegment(center, 10, 0, HALF_PI, upper_start, upper_end));
    EXPECT_FALSE(
        Environment::doesArcIntersectSegment(center, 10, 0, -HALF_PI, upper_start, upper_end));
}

/*
 *  Environment::doesArcIntersectSegment -- degenerate and boundary inputs
 */
TEST(DubinsBoundsTest, ArcSegmentEdgeCases) {
    initOpenField();
    const XYZCoord center(0, 0, 0);

    // segment entirely inside the circle never touches it
    EXPECT_FALSE(Environment::doesArcIntersectSegment(center, 10, 0, TWO_PI, XYZCoord(-1, -1, 0),
                                                      XYZCoord(1, 1, 0)));

    // zero length segment
    EXPECT_FALSE(Environment::doesArcIntersectSegment(center, 10, 0, TWO_PI, XYZCoord(10, 0, 0),
                                                      XYZCoord(10, 0, 0)));

    // tangent line y = 10 touches at exactly one point, (0, 10). Counted as a hit
    // (conservative), and only when the sweep covers 90 deg
    EXPECT_TRUE(Environment::doesArcIntersectSegment(center, 10, 0, M_PI, XYZCoord(-20, 10, 0),
                                                     XYZCoord(20, 10, 0)));
    EXPECT_FALSE(Environment::doesArcIntersectSegment(center, 10, M_PI, HALF_PI,
                                                      XYZCoord(-20, 10, 0), XYZCoord(20, 10, 0)));

    // segment whose endpoint lies exactly on the circle (t == 0)
    EXPECT_TRUE(Environment::doesArcIntersectSegment(center, 10, 0, HALF_PI, XYZCoord(10, 0, 0),
                                                     XYZCoord(30, 0, 0)));

    // a zero sweep arc is a single point, so it only touches the segment when that exact
    // point lies on it -- here, the tangent point at 90 deg
    EXPECT_TRUE(Environment::doesArcIntersectSegment(center, 10, HALF_PI, 0, XYZCoord(-20, 10, 0),
                                                     XYZCoord(20, 10, 0)));
    // ... and not when the arc collapses anywhere else on the circle
    EXPECT_FALSE(Environment::doesArcIntersectSegment(center, 10, 0, 0, XYZCoord(-20, 10, 0),
                                                      XYZCoord(20, 10, 0)));
}

/*
 *  Environment::isArcInBounds -- clear of everything
 */
TEST(DubinsBoundsTest, ArcInsideOpenField) {
    initOpenField();

    // full circle well inside the field
    EXPECT_TRUE(Environment::isArcInBounds(XYZCoord(50, 50, 0), 20, 0, TWO_PI));
    // small arcs anywhere in the interior
    EXPECT_TRUE(Environment::isArcInBounds(XYZCoord(20, 20, 0), 10, 0, HALF_PI));
    EXPECT_TRUE(Environment::isArcInBounds(XYZCoord(80, 80, 0), 10, M_PI, -M_PI));
}

/*
 *  Environment::isArcInBounds -- leaving the valid region
 */
TEST(DubinsBoundsTest, ArcLeavingRegionIsRejected) {
    initOpenField();

    // circle centered near the left edge, bulging out past x = 0
    EXPECT_FALSE(Environment::isArcInBounds(XYZCoord(5, 50, 0), 10, 0, TWO_PI));
    // the same circle, swept only on the side that stays inside
    EXPECT_TRUE(Environment::isArcInBounds(XYZCoord(5, 50, 0), 10, -HALF_PI, M_PI));

    // arc that starts outside the region entirely
    EXPECT_FALSE(Environment::isArcInBounds(XYZCoord(-50, 50, 0), 10, 0, HALF_PI));
}

/*
 *  Environment::isArcInBounds -- obstacles
 */
TEST(DubinsBoundsTest, ArcAndObstacle) {
    initFieldWithObstacle();

    // circle entirely inside the obstacle -- caught by the containment check
    EXPECT_FALSE(Environment::isArcInBounds(XYZCoord(50, 50, 0), 5, 0, TWO_PI));

    // circle that starts clear of the obstacle but cuts through it
    EXPECT_TRUE(Environment::isPointInBounds(XYZCoord(65, 30, 0)));  // arc start is clear
    EXPECT_FALSE(Environment::isArcInBounds(XYZCoord(50, 30, 0), 15, 0, TWO_PI));

    // same circle swept CCW from 180 through 270 deg, staying below y = 30 and so
    // never reaching the obstacle at y = 40
    EXPECT_TRUE(Environment::isArcInBounds(XYZCoord(50, 30, 0), 15, M_PI, M_PI));

    // circle sitting in the corner of the field, nowhere near the obstacle
    EXPECT_TRUE(Environment::isArcInBounds(XYZCoord(15, 15, 0), 10, 0, TWO_PI));
}

/*
 *  Environment::isArcInBounds -- differential against a densely sampled arc
 */
TEST(DubinsBoundsTest, ArcMatchesSampledReference) {
    initFieldWithObstacle();

    std::mt19937 gen(1337);
    std::uniform_real_distribution<double> coord(0, 100);
    std::uniform_real_distribution<double> radius(1, 40);
    std::uniform_real_distribution<double> angle(-TWO_PI, TWO_PI);

    int analytic_accepted = 0;
    for (int i = 0; i < 3000; i++) {
        const XYZCoord center(coord(gen), coord(gen), 0);
        const double r = radius(gen);
        const double start_angle = angle(gen);
        const double sweep = angle(gen);

        const bool analytic = Environment::isArcInBounds(center, r, start_angle, sweep);
        if (analytic) {
            analytic_accepted++;
            // the only direction that matters: never accept an arc that leaves bounds
            EXPECT_TRUE(arcInBoundsBruteForce(center, r, start_angle, sweep))
                << "accepted an out-of-bounds arc: center (" << center.x << ", " << center.y
                << ") r " << r << " from " << start_angle << " sweeping " << sweep;
        }
    }

    // guards against the check having degenerated into "always false"
    EXPECT_GT(analytic_accepted, 300);
}

/*
 *  Environment::isDubinsPathInBounds -- options that are rejected without any geometry
 */
TEST(DubinsBoundsTest, RejectsUnusableOptions) {
    initOpenField();
    Dubins::_radius = 5;
    Dubins::_point_separation = 0.5;

    const RRTPoint start(XYZCoord(50, 50, 0), 0);
    const RRTPoint end(XYZCoord(70, 50, 0), 0);

    // lsr/rsl hand back infinity when the turning circles overlap
    const RRTOption infinite{std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), true};
    EXPECT_FALSE(Environment::isDubinsPathInBounds(start, end, infinite));

    // CCC paths [LRL, RLR] are not handled analytically and are rejected outright
    const RRTOption curve_only{10, DubinsPath(1, 1, 1), false};
    EXPECT_FALSE(Environment::isDubinsPathInBounds(start, end, curve_only));
}

/*
 *  Environment::isDubinsPathInBounds -- endpoints outside the region
 */
TEST(DubinsBoundsTest, RejectsOutOfBoundsEndpoints) {
    initOpenField();
    Dubins::_radius = 5;
    Dubins::_point_separation = 0.5;

    const RRTPoint inside(XYZCoord(50, 50, 0), 0);
    const RRTPoint outside(XYZCoord(150, 50, 0), 0);

    for (const RRTOption& option : Dubins::allOptions(outside, inside)) {
        EXPECT_FALSE(Environment::isDubinsPathInBounds(outside, inside, option));
    }
    for (const RRTOption& option : Dubins::allOptions(inside, outside)) {
        EXPECT_FALSE(Environment::isDubinsPathInBounds(inside, outside, option));
    }
}

/*
 *  Environment::isDubinsPathInBounds -- short hops across open space are accepted
 */
TEST(DubinsBoundsTest, AcceptsPathInOpenField) {
    initOpenField();
    Dubins::_radius = 5;
    Dubins::_point_separation = 0.1;

    const RRTPoint start(XYZCoord(40, 50, 0), 0);
    const RRTPoint end(XYZCoord(60, 50, 0), 0);

    int accepted = 0;
    for (const RRTOption& option : Dubins::allOptions(start, end)) {
        if (Environment::isDubinsPathInBounds(start, end, option)) {
            accepted++;
            EXPECT_TRUE(dubinsInBoundsBruteForce(start, end, option));
        }
    }

    // a straight shot down the middle of an empty field: at least one option must work
    EXPECT_GT(accepted, 0);
}

/*
 *  Environment::isDubinsPathInBounds -- a turn that bulges out of the region
 *
 *  This is the case point-sampling was there to catch. Both endpoints are in bounds
 *  and the straight section is in bounds, but with a 20 unit turning radius the plane
 *  has to swing below y = 0 to reverse direction if it turns right.
 */
TEST(DubinsBoundsTest, RejectsTurnBulgingOutOfRegion) {
    initOpenField();
    Dubins::_radius = 20;
    Dubins::_point_separation = 0.05;

    const RRTPoint start(XYZCoord(50, 10, 0), 0);     // heading +x, near the bottom edge
    const RRTPoint end(XYZCoord(50, 30, 0), M_PI);    // heading -x

    ASSERT_TRUE(Environment::isPointInBounds(start.coord));
    ASSERT_TRUE(Environment::isPointInBounds(end.coord));

    // turning right puts the first turn's center at (50, -10), dragging the arc
    // below the bottom edge
    const RRTOption right = Dubins::rsr(start, end, Dubins::findCenter(start, 'R'),
                                        Dubins::findCenter(end, 'R'));
    EXPECT_FALSE(Environment::isDubinsPathInBounds(start, end, right));
    EXPECT_FALSE(dubinsInBoundsBruteForce(start, end, right));

    // reversing heading with a 20 unit radius needs ~100 units of room, so every option
    // here leaves the field somewhere -- the analytic verdict must track the sampled one
    for (const RRTOption& option : Dubins::allOptions(start, end)) {
        if (!std::isfinite(option.length)) {
            continue;
        }
        EXPECT_EQ(Environment::isDubinsPathInBounds(start, end, option),
                  dubinsInBoundsBruteForce(start, end, option));
    }

    // the identical maneuver with a tight turning radius fits inside the field
    Dubins::_radius = 3;
    const RRTOption tight = Dubins::rsr(start, end, Dubins::findCenter(start, 'R'),
                                        Dubins::findCenter(end, 'R'));
    EXPECT_TRUE(Environment::isDubinsPathInBounds(start, end, tight));
    EXPECT_TRUE(dubinsInBoundsBruteForce(start, end, tight));
}

/*
 *  Environment::isDubinsPathInBounds -- paths through an obstacle
 */
TEST(DubinsBoundsTest, RejectsPathThroughObstacle) {
    initFieldWithObstacle();
    Dubins::_radius = 5;
    Dubins::_point_separation = 0.05;

    // straight across the middle, which runs directly through [40, 60] x [40, 60]
    const RRTPoint start(XYZCoord(10, 50, 0), 0);
    const RRTPoint end(XYZCoord(90, 50, 0), 0);

    for (const RRTOption& option : Dubins::allOptions(start, end)) {
        EXPECT_FALSE(Environment::isDubinsPathInBounds(start, end, option))
            << "accepted a path straight through the obstacle";
    }

    // the same hop, moved down to y = 20 where the obstacle is not in the way
    const RRTPoint clear_start(XYZCoord(10, 20, 0), 0);
    const RRTPoint clear_end(XYZCoord(90, 20, 0), 0);

    int accepted = 0;
    for (const RRTOption& option : Dubins::allOptions(clear_start, clear_end)) {
        if (Environment::isDubinsPathInBounds(clear_start, clear_end, option)) {
            accepted++;
        }
    }
    EXPECT_GT(accepted, 0);
}

/*
 *  Environment::isDubinsPathInBounds -- differential against the sampled check,
 *  over every option Dubins::allOptions produces for random start/end pairs
 */
TEST(DubinsBoundsTest, MatchesSampledReference) {
    initFieldWithObstacle();
    Dubins::_radius = 5;
    Dubins::_point_separation = 0.02;  // fine enough for the reference to be trusted

    std::mt19937 gen(42);
    std::uniform_real_distribution<double> position(2, 98);
    std::uniform_real_distribution<double> heading(0, TWO_PI);

    int total = 0;
    int agree = 0;
    int accepted = 0;

    for (int i = 0; i < 750; i++) {
        const RRTPoint start(XYZCoord(position(gen), position(gen), 0), heading(gen));
        const RRTPoint end(XYZCoord(position(gen), position(gen), 0), heading(gen));

        for (const RRTOption& option : Dubins::allOptions(start, end)) {
            if (!std::isfinite(option.length)) {
                continue;
            }

            const bool analytic = Environment::isDubinsPathInBounds(start, end, option);
            const bool sampled = dubinsInBoundsBruteForce(start, end, option);

            total++;
            agree += (analytic == sampled);
            accepted += analytic;

            // the check may reject a path the sampler accepts (grazing a boundary between
            // samples), but must never accept one the sampler rejects
            EXPECT_FALSE(analytic && !sampled)
                << "accepted an out-of-bounds path: start (" << start.coord.x << ", "
                << start.coord.y << ") psi " << start.psi << " -> end (" << end.coord.x << ", "
                << end.coord.y << ") psi " << end.psi;
        }
    }

    ASSERT_GT(total, 0);
    // conservative rejections are fine, but they should be rare
    EXPECT_GT(static_cast<double>(agree) / total, 0.99);
    // and the check must not have degenerated into "always false"
    EXPECT_GT(accepted, total / 10);
}

/*
 *  Environment::isDubinsPathInBounds -- holds up across turning radii and obstacle layouts
 */
TEST(DubinsBoundsTest, MatchesSampledReferenceAcrossConfigurations) {
    Polygon field = {{XYZCoord(0, 0, 0), XYZCoord(100, 0, 0), XYZCoord(100, 100, 0),
                      XYZCoord(0, 100, 0)}};

    // a thin wall, a corner block, and a pair of pillars
    const std::vector<std::vector<Polygon>> obstacle_sets = {
        {},
        {{{XYZCoord(48, 10, 0), XYZCoord(52, 10, 0), XYZCoord(52, 90, 0), XYZCoord(48, 90, 0)}}},
        {{{XYZCoord(0, 0, 0), XYZCoord(30, 0, 0), XYZCoord(30, 30, 0), XYZCoord(0, 30, 0)}}},
        {{{XYZCoord(20, 20, 0), XYZCoord(35, 20, 0), XYZCoord(35, 35, 0), XYZCoord(20, 35, 0)}},
         {{XYZCoord(65, 65, 0), XYZCoord(80, 65, 0), XYZCoord(80, 80, 0), XYZCoord(65, 80, 0)}}},
    };

    std::mt19937 gen(2024);
    std::uniform_real_distribution<double> position(2, 98);
    std::uniform_real_distribution<double> heading(0, TWO_PI);

    for (const double turn_radius : {2.0, 5.0, 15.0, 30.0}) {
        for (const std::vector<Polygon>& obstacles : obstacle_sets) {
            Environment::init(field, {}, {}, obstacles);
            Dubins::_radius = turn_radius;
            Dubins::_point_separation = 0.02;

            for (int i = 0; i < 120; i++) {
                const RRTPoint start(XYZCoord(position(gen), position(gen), 0), heading(gen));
                const RRTPoint end(XYZCoord(position(gen), position(gen), 0), heading(gen));

                for (const RRTOption& option : Dubins::allOptions(start, end)) {
                    if (!std::isfinite(option.length)) {
                        continue;
                    }

                    if (Environment::isDubinsPathInBounds(start, end, option)) {
                        EXPECT_TRUE(dubinsInBoundsBruteForce(start, end, option))
                            << "radius " << turn_radius << ", " << obstacles.size()
                            << " obstacle(s): accepted an out-of-bounds path from ("
                            << start.coord.x << ", " << start.coord.y << ") to (" << end.coord.x
                            << ", " << end.coord.y << ")";
                    }
                }
            }
        }
    }
}
