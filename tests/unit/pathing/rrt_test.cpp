#include "pathing/rrt.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "pathing/tree.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

// the state rand_r() is walked from, so a test that samples can be repeated
extern unsigned int seed1;

namespace {

void seedRandom(unsigned int seed) { seed1 = seed; }

static inline void setDubins(double r, double sep) {
    Dubins::_radius = r;
    Dubins::_point_separation = sep;
}

// 1000 x 1000 field, nothing in it. A 30m turning radius leaves plenty of room
const Polygon FIELD = {{XYZCoord(0, 0, 0), XYZCoord(1000, 0, 0), XYZCoord(1000, 1000, 0),
                        XYZCoord(0, 1000, 0)}};

void initOpenField() {
    Environment::init(FIELD, {}, {}, {});
    setDubins(30, 10);
}

// a wall that splits the field at x in [480, 520], with a 300m gap at the top
const Polygon WALL = {{XYZCoord(480, 0, 0), XYZCoord(520, 0, 0), XYZCoord(520, 700, 0),
                       XYZCoord(480, 700, 0)}};

void initFieldWithWall() {
    Environment::init(FIELD, {}, {}, {WALL});
    setDubins(30, 10);
}

// the cheapest connection from anywhere in the tree to any of the given points
// that can actually be flown, found by brute force
Connection cheapestFlyableConnection(const RRT& rrt, const std::vector<RRTPoint>& ends) {
    Connection best;

    for (NodeId node = 0; node < rrt.tree.tree.size; node++) {
        const RRTPoint& anchor = rrt.tree.tree.points[node];

        for (const RRTPoint& end : ends) {
            for (const RRTOption& option : Dubins::allOptions(anchor, end)) {
                const double cost = rrt.tree.tree.length[node] + option.length;

                if (std::isfinite(cost) && cost < best.cost &&
                    Environment::isDubinsPathInBounds(anchor, end, option)) {
                    best = {node, end, option, cost};
                }
            }
        }
    }

    return best;
}

bool pathIsInBounds(const std::vector<XYZCoord>& path) {
    for (const XYZCoord& point : path) {
        if (!Environment::isPointInBounds(point)) {
            return false;
        }
    }
    return true;
}

// the index of the first point of the path that lands on a waypoint, searching
// from `from` so that waypoints can be checked in the order they are flown
std::size_t indexOfPoint(const std::vector<XYZCoord>& path, const XYZCoord& target,
                         std::size_t from = 0) {
    for (std::size_t i = from; i < path.size(); i++) {
        if (std::hypot(path[i].x - target.x, path[i].y - target.y) < 1e-6) {
            return i;
        }
    }
    return path.size();
}

// the heading the path is flying as it lands on the point at `index`
double headingAt(const std::vector<XYZCoord>& path, std::size_t index) {
    const XYZCoord& previous = path[index - 1];
    return std::atan2(path[index].y - previous.y, path[index].x - previous.x);
}

// how far apart two headings are, the short way around
double angleBetween(double a, double b) {
    return std::abs(std::remainder(a - b, TWO_PI));
}

}  // namespace

/*
 *  A fresh RRT holds nothing but the plane's current vector, which is the first
 *  of the points it flies through
 */
TEST(RRTTest, ConstructionSeedsTheTreeWithTheStart) {
    initOpenField();
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(500, 500, 30)};

    const RRT rrt(goals, HALF_PI);

    // the tree is rooted where the plane is, flying the heading it was given
    EXPECT_TRUE(rrt.tree.getStart() == RRTPoint(goals[0], HALF_PI));
    EXPECT_EQ(rrt.tree.tree.size, 1);
    EXPECT_TRUE(rrt.getPointsToGoal().empty());
    EXPECT_EQ(rrt.goals, goals);

    // every approach angle is tried at every goal unless the caller asks for a
    // specific set, and the plane's own heading stands in for the goal it is on
    ASSERT_EQ(rrt.goal_angles.size(), goals.size());
    EXPECT_EQ(rrt.goal_angles[0], std::vector<double>({HALF_PI}));
    EXPECT_EQ(rrt.goal_angles[1], DEFAULT_GOAL_ANGLES);

    const std::vector<double> angles = {0.0, M_PI};
    const RRT custom_angles(goals, HALF_PI, angles);
    EXPECT_EQ(custom_angles.goal_angles[1], angles);
}

/*
 *  A caller that dictates how a goal is approached -- coverage pathing does, as a
 *  scan line has to be flown along its own direction -- leaves it a single angle
 */
TEST(RRTTest, ConstructionPinsTheGoalsTheCallerNamedAnAngleFor) {
    initOpenField();
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(500, 500, 30),
                                         XYZCoord(800, 200, 30)};
    const std::vector<std::vector<double>> goal_angles = {{}, {0}, {M_PI}};

    const RRT rrt(goals, HALF_PI, goal_angles);

    EXPECT_TRUE(rrt.tree.getStart() == RRTPoint(goals[0], HALF_PI));
    EXPECT_EQ(rrt.tree.tree.size, 1);
    EXPECT_EQ(rrt.goals, goals);

    // whatever the caller put down for the goal the plane is already sitting on,
    // the heading it reached that one at is the one it is flying
    EXPECT_EQ(rrt.goal_angles[0], std::vector<double>({HALF_PI}));
    EXPECT_EQ(rrt.goal_angles[1], std::vector<double>({0}));
    EXPECT_EQ(rrt.goal_angles[2], std::vector<double>({M_PI}));
}

/*
 *  RRT::goalEndpoints -- a goal left a single angle is only reachable the one way
 */
TEST(RRTTest, GoalEndpointsHonorAPinnedAngle) {
    initOpenField();
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(500, 500, 30),
                                         XYZCoord(800, 200, 30)};
    const std::vector<std::vector<double>> goal_angles = {{}, {HALF_PI}, {M_PI}};

    const RRT rrt(goals, 0, goal_angles);

    // the plane sits on the first of them, it is flown from and never to
    for (std::size_t goal = 1; goal < goals.size(); goal++) {
        const std::vector<RRTPoint> ends = rrt.goalEndpoints(goal);

        ASSERT_EQ(ends.size(), 1);
        EXPECT_TRUE(ends[0] == RRTPoint(goals[goal], goal_angles[goal][0]));
    }
}

/*
 *  RRT::run -- the way between the waypoints is up to RRT, but the heading a pinned
 *  one is reached at is not
 */
TEST(RRTTest, RunReachesEveryWaypointAtItsPinnedAngle) {
    initOpenField();
    seedRandom(7);

    // scan lines, the way coverage pathing lays them out: swept one way, then back
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(200, 300, 30),
                                         XYZCoord(800, 300, 30), XYZCoord(800, 400, 30),
                                         XYZCoord(200, 400, 30)};
    const std::vector<std::vector<double>> goal_angles = {{}, {0}, {0}, {M_PI}, {M_PI}};

    RRT rrt(goals, 0, goal_angles);
    rrt.run();

    const std::vector<XYZCoord> path = rrt.getPointsToGoal();
    ASSERT_FALSE(path.empty());
    EXPECT_TRUE(pathIsInBounds(path));

    std::size_t index = 0;
    for (std::size_t goal = 1; goal < goals.size(); goal++) {
        index = indexOfPoint(path, goals[goal], index);
        ASSERT_LT(index, path.size()) << "path never reached waypoint " << goal;
        ASSERT_GT(index, 0);

        // the points are far enough apart that the last leg of an arc only
        // approximates the heading it lands on
        EXPECT_LT(angleBetween(headingAt(path, index), goal_angles[goal][0]), 0.25)
            << "waypoint " << goal << " was not flown at the angle it was pinned to";
        EXPECT_NEAR(path[index].z, goals[goal].z, 1e-9);
    }

    EXPECT_EQ(indexOfPoint(path, goals.back(), index), path.size() - 1);
}

/*
 *  RRT::generateDubinsOptions finds the paths and stops there -- what they cost is
 *  known without flying them, so a mission can be weighed against another one and
 *  thrown away without ever generating a point
 */
TEST(RRTTest, DubinsOptionsAreFoundWithoutFlyingThem) {
    initOpenField();
    seedRandom(13);
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(400, 300, 30),
                                         XYZCoord(800, 600, 30)};

    RRT rrt(goals, 0);
    EXPECT_EQ(rrt.pathLength(), 0);
    EXPECT_TRUE(rrt.getPointsToGoal().empty());

    rrt.generateDubinsOptions();

    // one leg per waypoint flown to, each landing on the goal it was found for
    ASSERT_EQ(rrt.legs.size(), goals.size() - 1);
    for (std::size_t i = 0; i < rrt.legs.size(); i++) {
        EXPECT_EQ(rrt.legs[i].goal_idx, i + 1);
        EXPECT_FALSE(rrt.legs[i].segments.empty());
        EXPECT_TRUE(rrt.legs[i].segments.back().end.coord == goals[i + 1]);
        EXPECT_GT(rrt.legs[i].length, 0);
    }

    // the legs start where the one behind them landed
    EXPECT_TRUE(rrt.legs[0].start == RRTPoint(goals[0], 0));
    EXPECT_TRUE(rrt.legs[1].start == rrt.legs[0].segments.back().end);

    // how long the mission is is known, but not one point of it has been flown
    double straight_line = 0;
    for (std::size_t i = 1; i < goals.size(); i++) {
        straight_line += std::hypot(goals[i].x - goals[i - 1].x, goals[i].y - goals[i - 1].y);
    }
    EXPECT_GE(rrt.pathLength(), straight_line);
    EXPECT_TRUE(rrt.getPointsToGoal().empty());

    rrt.generateFlightPoints();

    const std::vector<XYZCoord> path = rrt.getPointsToGoal();
    ASSERT_FALSE(path.empty());

    double flown = goals[0].distanceTo(path[0]);
    for (std::size_t i = 1; i < path.size(); i++) {
        flown += std::hypot(path[i].x - path[i - 1].x, path[i].y - path[i - 1].y);
    }

    // the points cut the corners off the arcs, so they cover a little less ground
    // than the legs they came from
    EXPECT_LT(flown, rrt.pathLength());
    EXPECT_GT(flown, rrt.pathLength() * 0.95);

    // flying the legs a second time does not append the mission to itself
    rrt.generateFlightPoints();
    EXPECT_EQ(rrt.getPointsToGoal().size(), path.size());

    // and run() is the two of them, one after the other
    RRT ran(goals, 0);
    seedRandom(13);
    ran.run();
    EXPECT_EQ(ran.legs.size(), rrt.legs.size());
    EXPECT_EQ(ran.getPointsToGoal().size(), path.size());
}

/*
 *  RRT::fillOptions -- every dubins path that exists from one node, and nothing else
 */
TEST(RRTTest, FillOptionsCollectsTheFlyablePathsFromANode) {
    initOpenField();
    const RRTPoint start(XYZCoord(100, 100, 0), 0);
    const RRT rrt({start.coord, XYZCoord(500, 500, 30)}, start.psi);

    const RRTPoint end(XYZCoord(500, 500, 0), HALF_PI);
    rrt.fillOptions(0, {end});

    // all four of the CSC paths exist between two vectors this far apart
    EXPECT_EQ(rrt.options.size(), 4);
    for (const Connection& option : rrt.options) {
        EXPECT_EQ(option.anchor, 0);
        EXPECT_TRUE(option.end == end);
        EXPECT_TRUE(std::isfinite(option.option.length));

        // the root has nothing behind it, so the flight is the path itself
        EXPECT_DOUBLE_EQ(option.cost, option.option.length);
    }

    // the scratch space is written over, not appended to
    rrt.fillOptions(0, {end});
    EXPECT_EQ(rrt.options.size(), 4);

    // every endpoint asked for is pathed to
    const RRTPoint other_end(XYZCoord(400, 600, 0), 0);
    rrt.fillOptions(0, {end, other_end});
    EXPECT_EQ(rrt.options.size(), 8);

    // paths that do not exist are dropped -- the turning circles of these two
    // vectors overlap, so there is no RSL path between them
    rrt.fillOptions(0, {RRTPoint(XYZCoord(100, 80, 0), 0)});
    EXPECT_EQ(rrt.options.size(), 3);
}

/*
 *  RRT::lowerBound -- a flight through a node never costs less than the ground it
 *  has to cover, which is what lets the search skip nodes it has not pathed from
 */
TEST(RRTTest, LowerBoundNeverExceedsWhatAFlightCosts) {
    initOpenField();
    RRT rrt({XYZCoord(100, 100, 0), XYZCoord(900, 900, 30)}, 0);

    // a couple of branches, so the nodes sit at different distances from the root
    const RRTPoint near_node(XYZCoord(200, 150, 0), 0);
    const RRTPoint far_node(XYZCoord(700, 200, 0), HALF_PI);
    rrt.tree.addSample(0, near_node, Dubins::bestOption(rrt.tree.getStart(), near_node));
    rrt.tree.addSample(1, far_node, Dubins::bestOption(near_node, far_node));
    ASSERT_EQ(rrt.tree.tree.size, 3);

    const std::vector<RRTPoint> ends = {RRTPoint(XYZCoord(800, 800, 0), HALF_PI),
                                        RRTPoint(XYZCoord(300, 900, 0), 0)};

    for (NodeId node = 0; node < rrt.tree.tree.size; node++) {
        const double bound = rrt.lowerBound(node, ends);

        // the flight to the node itself is already paid for
        EXPECT_GE(bound, rrt.tree.tree.length[node]);

        rrt.fillOptions(node, ends);
        ASSERT_FALSE(rrt.options.empty());

        for (const Connection& option : rrt.options) {
            EXPECT_LE(bound, option.cost);
        }
    }
}

/*
 *  RRT::bestConnection -- a path that leaves the airspace is not one that can be
 *  taken, no matter how cheap it is
 */
TEST(RRTTest, BestConnectionStaysInsideTheAirspace) {
    initFieldWithWall();
    const RRTPoint start(XYZCoord(200, 400, 0), 0);
    RRT rrt({start.coord, XYZCoord(900, 100, 30)}, start.psi);

    // the plane is boxed in against the wall, so the way to some of these is not
    // the shortest one
    const std::vector<RRTPoint> targets = {RRTPoint(XYZCoord(400, 200, 0), M_PI),
                                           RRTPoint(XYZCoord(300, 800, 0), HALF_PI),
                                           RRTPoint(XYZCoord(120, 400, 0), M_PI),
                                           RRTPoint(XYZCoord(460, 650, 0), 0)};

    for (const RRTPoint& end : targets) {
        const Connection connection = rrt.bestConnection({end}, TOTAL_OPTIONS_FOR_GOAL_CONNECTION);

        if (connection.isValid()) {
            EXPECT_TRUE(Environment::isDubinsPathInBounds(rrt.tree.tree.points[connection.anchor],
                                                          connection.end, connection.option));
        }

        // everything cheaper than what it settled on cuts through the wall or the
        // edge of the field
        rrt.fillOptions(0, {end});
        ASSERT_FALSE(rrt.options.empty());

        for (const Connection& option : rrt.options) {
            if (option.cost < connection.cost) {
                EXPECT_FALSE(Environment::isDubinsPathInBounds(
                    rrt.tree.tree.points[option.anchor], option.end, option.option))
                    << "passed up a cheaper path to (" << end.coord.x << ", " << end.coord.y << ")";
            }
        }
    }
}

/*
 *  RRT::bestConnection -- the cheapest flight to the point that can be flown, and
 *  it is not committed to the tree
 */
TEST(RRTTest, BestConnectionIsTheCheapestOneThatCanBeFlown) {
    initOpenField();
    RRT rrt({XYZCoord(100, 100, 0), XYZCoord(900, 900, 30)}, 0);

    // a couple of branches, so the nodes sit at different distances from the root
    const RRTPoint near_node(XYZCoord(200, 150, 0), 0);
    const RRTPoint far_node(XYZCoord(700, 200, 0), HALF_PI);
    rrt.tree.addSample(0, near_node, Dubins::bestOption(rrt.tree.getStart(), near_node));
    rrt.tree.addSample(1, far_node, Dubins::bestOption(near_node, far_node));
    ASSERT_EQ(rrt.tree.tree.size, 3);

    const std::vector<RRTPoint> ends = {RRTPoint(XYZCoord(800, 800, 0), HALF_PI)};
    const Connection connection = rrt.bestConnection(ends, TOTAL_OPTIONS_FOR_GOAL_CONNECTION);
    const Connection cheapest = cheapestFlyableConnection(rrt, ends);

    ASSERT_TRUE(connection.isValid());
    EXPECT_EQ(connection.anchor, cheapest.anchor);
    EXPECT_DOUBLE_EQ(connection.cost, cheapest.cost);
    EXPECT_TRUE(connection.end == ends[0]);
    EXPECT_DOUBLE_EQ(connection.cost,
                     rrt.tree.tree.length[connection.anchor] + connection.option.length);

    // the tree is left alone -- the caller decides whether to commit
    EXPECT_EQ(rrt.tree.tree.size, 3);
}

/*
 *  RRT::bestConnection -- any of the points will do, and the cheapest one wins
 */
TEST(RRTTest, BestConnectionTakesTheCheapestOfTheEndpoints) {
    initOpenField();
    RRT rrt({XYZCoord(100, 100, 0), XYZCoord(900, 900, 30)}, 0);

    // straight ahead of the plane, and well off to the side of it
    const RRTPoint close(XYZCoord(300, 100, 0), 0);
    const RRTPoint distant(XYZCoord(800, 700, 0), M_PI);

    const Connection connection =
        rrt.bestConnection({distant, close}, TOTAL_OPTIONS_FOR_GOAL_CONNECTION);

    ASSERT_TRUE(connection.isValid());
    EXPECT_TRUE(connection.end == close);
    EXPECT_DOUBLE_EQ(connection.cost,
                     cheapestFlyableConnection(rrt, {distant, close}).cost);
}

/*
 *  RRT::bestConnection -- an unreachable point hands back an invalid connection
 */
TEST(RRTTest, BestConnectionGivesUpOnAnUnreachablePoint) {
    initOpenField();
    RRT rrt({XYZCoord(100, 100, 0), XYZCoord(900, 900, 30)}, 0);

    const Connection connection = rrt.bestConnection({RRTPoint(XYZCoord(2000, 2000, 0), 0)},
                                                     TOTAL_OPTIONS_FOR_GOAL_CONNECTION);

    EXPECT_FALSE(connection.isValid());
    EXPECT_EQ(connection.anchor, INVALID_NODE);
    EXPECT_FALSE(std::isfinite(connection.cost));
}

/*
 *  RRT::bestConnection -- nothing on the other side of the wall is reachable, and
 *  the search does not check more paths than it is allowed to
 */
TEST(RRTTest, BestConnectionStopsOnceItHasCheckedItsBudget) {
    initFieldWithWall();
    RRT rrt({XYZCoord(200, 400, 0), XYZCoord(900, 100, 30)}, 0);

    // every way to the other side of the wall goes through it
    const RRTPoint across(XYZCoord(800, 400, 0), 0);
    EXPECT_FALSE(rrt.bestConnection({across}, TOTAL_OPTIONS_FOR_GOAL_CONNECTION).isValid());

    // a point that is reachable is still missed if no path may be checked at all
    const RRTPoint reachable(XYZCoord(400, 200, 0), M_PI);
    EXPECT_TRUE(rrt.bestConnection({reachable}, TOTAL_OPTIONS_FOR_GOAL_CONNECTION).isValid());
    EXPECT_FALSE(rrt.bestConnection({reachable}, 0).isValid());
}

/*
 *  RRT::goalEndpoints -- the goal is tried at every approach angle
 */
TEST(RRTTest, GoalEndpointsCoverEveryApproachAngle) {
    initOpenField();
    const std::vector<double> angles = {0.0, HALF_PI, M_PI};
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(500, 500, 30),
                                         XYZCoord(800, 200, 45)};
    RRT rrt(goals, 0, angles);

    // the plane sits on the first of them, it is flown from and never to
    for (std::size_t goal = 1; goal < goals.size(); goal++) {
        const std::vector<RRTPoint> ends = rrt.goalEndpoints(goal);

        ASSERT_EQ(ends.size(), angles.size());
        for (std::size_t i = 0; i < ends.size(); i++) {
            EXPECT_TRUE(ends[i].coord == goals[goal]);
            EXPECT_DOUBLE_EQ(ends[i].psi, angles[i]);
        }
    }
}

/*
 *  RRT::connectToGoal -- the flight path is extended and the tree restarts at the
 *  waypoint that was just reached
 */
TEST(RRTTest, ConnectToGoalCommitsThePathAndRestartsTheTree) {
    initOpenField();
    const XYZCoord goal(500, 500, 30);
    RRT rrt({XYZCoord(100, 100, 0), goal}, 0);

    ASSERT_TRUE(rrt.connectToGoal(1));
    rrt.generateFlightPoints();

    const std::vector<XYZCoord> path = rrt.getPointsToGoal();
    ASSERT_FALSE(path.empty());
    EXPECT_TRUE(pathIsInBounds(path));
    EXPECT_NEAR(path.back().x, goal.x, 1e-6);
    EXPECT_NEAR(path.back().y, goal.y, 1e-6);

    // the waypoint is the root of the tree the next waypoint is pathed from
    EXPECT_EQ(rrt.tree.tree.size, 1);
    EXPECT_TRUE(rrt.tree.getStart().coord == goal);
    EXPECT_NE(std::find(rrt.goal_angles[1].begin(), rrt.goal_angles[1].end(),
                        rrt.tree.getStart().psi),
              rrt.goal_angles[1].end());
}

/*
 *  RRT::connectToGoal -- a goal outside the airspace is never connected to
 */
TEST(RRTTest, ConnectToGoalFailsOnAnUnreachableGoal) {
    initOpenField();
    const RRTPoint start(XYZCoord(100, 100, 0), 0);
    RRT rrt({start.coord, XYZCoord(2000, 2000, 30)}, start.psi);

    EXPECT_FALSE(rrt.connectToGoal(1));
    EXPECT_TRUE(rrt.getPointsToGoal().empty());

    // the tree is left as it was, still rooted at the plane
    EXPECT_EQ(rrt.tree.tree.size, 1);
    EXPECT_TRUE(rrt.tree.getStart() == start);
}

/*
 *  RRT::commitConnection -- the plane climbs at a constant rate along the ground it
 *  covers, so the altitude is interpolated over the length of the segment
 */
TEST(RRTTest, CommitConnectionClimbsToTheWaypointAltitude) {
    initOpenField();
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(500, 500, 100),
                                         XYZCoord(800, 200, 50)};
    RRT rrt(goals, 0);

    // first leg: climbs from the plane's altitude (0) up to 100
    const RRTPoint first_goal(goals[1], 0);
    rrt.commitConnection({0, first_goal, Dubins::bestOption(rrt.tree.getStart(), first_goal), 0},
                         1);
    rrt.generateFlightPoints();

    const std::vector<XYZCoord> first_leg = rrt.getPointsToGoal();
    ASSERT_FALSE(first_leg.empty());
    EXPECT_NEAR(first_leg.back().z, 100, 1e-9);
    EXPECT_GT(first_leg.front().z, 0);
    EXPECT_LT(first_leg.front().z, 100);
    for (std::size_t i = 1; i < first_leg.size(); i++) {
        EXPECT_GE(first_leg[i].z, first_leg[i - 1].z);
    }

    // second leg: descends from the altitude of the waypoint behind it, not from
    // the altitude the plane started the mission at
    const RRTPoint second_goal(goals[2], 0);
    rrt.commitConnection({0, second_goal, Dubins::bestOption(rrt.tree.getStart(), second_goal), 0},
                         2);
    rrt.generateFlightPoints();

    const std::vector<XYZCoord> path = rrt.getPointsToGoal();
    ASSERT_GT(path.size(), first_leg.size());
    EXPECT_NEAR(path.back().z, 50, 1e-9);

    for (std::size_t i = first_leg.size(); i < path.size(); i++) {
        EXPECT_LE(path[i].z, 100);
        EXPECT_GE(path[i].z, 50);
        EXPECT_LE(path[i].z, path[i - 1].z);
    }
}

/*
 *  RRT::RRTIteration -- the goal is connected to once the sampling is done
 */
TEST(RRTTest, RRTIterationConnectsToAReachableGoal) {
    initOpenField();
    seedRandom(11);
    const XYZCoord goal(700, 700, 30);
    RRT rrt({XYZCoord(100, 100, 0), goal}, 0);

    EXPECT_TRUE(rrt.RRTIteration(1));
    rrt.generateFlightPoints();

    const std::vector<XYZCoord> path = rrt.getPointsToGoal();
    ASSERT_FALSE(path.empty());
    EXPECT_TRUE(pathIsInBounds(path));
    EXPECT_TRUE(rrt.tree.getStart().coord == goal);
}

/*
 *  RRT::run -- every waypoint is flown, in order
 */
TEST(RRTTest, RunFliesEveryWaypointInOrder) {
    initOpenField();
    seedRandom(3);
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(400, 300, 30),
                                         XYZCoord(800, 600, 45), XYZCoord(200, 800, 60)};
    RRT rrt(goals, 0);

    rrt.run();

    const std::vector<XYZCoord> path = rrt.getPointsToGoal();
    ASSERT_FALSE(path.empty());
    EXPECT_TRUE(pathIsInBounds(path));

    // the plane is already on the first of them, the path is what it flies after
    std::size_t index = 0;
    for (std::size_t goal = 1; goal < goals.size(); goal++) {
        index = indexOfPoint(path, goals[goal], index);
        ASSERT_LT(index, path.size())
            << "path never reached (" << goals[goal].x << ", " << goals[goal].y << ")";
        EXPECT_NEAR(path[index].z, goals[goal].z, 1e-9);
    }

    // the path ends on the last waypoint, and so does the tree
    EXPECT_EQ(indexOfPoint(path, goals.back(), index), path.size() - 1);
    EXPECT_TRUE(rrt.tree.getStart().coord == goals.back());
}

/*
 *  RRT::run -- the path found around an obstacle stays in bounds the whole way
 */
TEST(RRTTest, RunPathsAroundAnObstacle) {
    initFieldWithWall();
    seedRandom(23);
    const XYZCoord goal(800, 400, 30);
    RRT rrt({XYZCoord(200, 400, 0), goal}, 0);

    rrt.run();

    const std::vector<XYZCoord> path = rrt.getPointsToGoal();
    ASSERT_FALSE(path.empty()) << "never made it around the wall";
    EXPECT_TRUE(pathIsInBounds(path));

    // it got to the other side, and the only way across is the gap above the wall
    EXPECT_LT(indexOfPoint(path, goal), path.size());

    // a straightaway is described by its two endpoints alone, so checking the
    // points is not enough -- no leg of the path may cut through the wall
    for (std::size_t i = 1; i < path.size(); i++) {
        EXPECT_FALSE(Environment::doesLineIntersectPolygon(path[i - 1], path[i], WALL))
            << "leg " << i << " cuts through the wall";
    }
}

/*
 *  RRT::bestConnection -- the pruning is what makes the search cheap, so whatever
 *  the tree looks like and wherever the goal is, it has to come back with the same
 *  connection an exhaustive check would
 */
TEST(RRTTest, BestConnectionMatchesAnExhaustiveSearch) {
    initFieldWithWall();
    seedRandom(101);

    const XYZCoord start(200, 400, 0);

    for (int trial = 0; trial < 25; trial++) {
        // the goal is on the far side of the wall half of the time, so the search
        // is made to give up as well as to succeed
        const XYZCoord goal = (trial % 2 == 0) ? XYZCoord(300, 800, 30) : XYZCoord(900, 100, 30);
        RRT rrt({start, goal}, 0);

        // a tree of random samples, grown the way an iteration would grow it
        for (int i = 0; i < 40; i++) {
            const RRTPoint sample(Environment::getRandomPoint(false, start), random(0, TWO_PI));
            const Connection connection = rrt.bestConnection({sample}, MAX_DUBINS_OPTIONS_TO_PARSE);

            if (connection.isValid()) {
                rrt.tree.addSample(connection.anchor, connection.end, connection.option);
            }
        }
        ASSERT_GT(rrt.tree.tree.size, 1) << "trial " << trial << " grew nothing to search";

        const std::vector<RRTPoint> ends = rrt.goalEndpoints(1);
        const Connection found = rrt.bestConnection(ends, std::numeric_limits<int>::max());
        const Connection exhaustive = cheapestFlyableConnection(rrt, ends);

        ASSERT_EQ(found.isValid(), exhaustive.isValid()) << "trial " << trial;

        if (found.isValid()) {
            EXPECT_DOUBLE_EQ(found.cost, exhaustive.cost) << "trial " << trial;
            EXPECT_EQ(found.anchor, exhaustive.anchor) << "trial " << trial;
        }
    }
}
