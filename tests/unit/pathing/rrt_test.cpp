#include "pathing/rrt.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

#include "field.hpp"
#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "pathing/tree.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

namespace {

// the cheapest connection from anywhere in the tree to any of the given points
// that can actually be flown, found by brute force -- the node it hangs off of and
// what the flight costs, INVALID_NODE if none of them can be reached
std::pair<NodeId, double> cheapestFlyableConnection(const RRT& rrt,
                                                    const std::vector<RRTPoint>& ends) {
    NodeId best_anchor = INVALID_NODE;
    double best_cost = std::numeric_limits<double>::infinity();

    for (NodeId node = 0; node < rrt.tree.size; node++) {
        const RRTPoint anchor = rrt.tree.point(node);

        for (const RRTPoint& end : ends) {
            for (const DubinsPath& path : Dubins::allOptions(anchor, end)) {
                const double cost = rrt.tree.length[node] + path.length;

                if (std::isfinite(cost) && cost < best_cost &&
                    Environment::isDubinsPathInBounds(anchor, end, path)) {
                    best_anchor = node;
                    best_cost = cost;
                }
            }
        }
    }

    return {best_anchor, best_cost};
}

// what a connection off the given node costs, measured from the root of the tree
double costOf(const RRT& rrt, NodeId anchor, const PathSegment& segment) {
    return rrt.tree.length[anchor] + segment.path.length;
}

// the points a leg is flown along, which RRT hands back as dubins paths alone
std::vector<XYZCoord> flyLeg(const Leg& leg) {
    return Dubins::generatePath(leg.start, leg.segments);
}

}  // namespace

/*
 *  A fresh RRT holds nothing but the vector it is rooted at, which is where every
 *  leg it finds is flown from
 */
TEST(RRTTest, ConstructionSeedsTheTreeWithTheStart) {
    initOpenField();
    const RRTPoint start(XYZCoord(100, 100, 0), HALF_PI);

    const RRT rrt(start);

    EXPECT_TRUE(rrt.tree.getStart() == start);
    EXPECT_EQ(rrt.tree.size, 1);
}

/*
 *  goalEndpoints -- a goal left a single angle is only reachable the one way
 */
TEST(RRTTest, GoalEndpointsHonorAPinnedAngle) {
    const XYZCoord goal(500, 500, 30);

    const std::vector<RRTPoint> ends = goalEndpoints(goal, {HALF_PI});

    ASSERT_EQ(ends.size(), 1);
    EXPECT_TRUE(ends[0] == RRTPoint(goal, HALF_PI));
}

/*
 *  goalEndpoints -- the goal is tried at every approach angle
 */
TEST(RRTTest, GoalEndpointsCoverEveryApproachAngle) {
    const std::vector<double> angles = {0.0, HALF_PI, M_PI};
    const XYZCoord goal(800, 200, 45);

    const std::vector<RRTPoint> ends = goalEndpoints(goal, angles);

    ASSERT_EQ(ends.size(), angles.size());
    for (std::size_t i = 0; i < ends.size(); i++) {
        EXPECT_TRUE(ends[i].coord == goal);
        EXPECT_DOUBLE_EQ(ends[i].psi, angles[i]);
    }
}

/*
 *  RRT::fillOptions -- every dubins path that exists from one node, and nothing else
 */
TEST(RRTTest, FillOptionsCollectsTheFlyablePathsFromANode) {
    initOpenField();
    const RRT rrt(RRTPoint(XYZCoord(100, 100, 0), 0));

    const RRTPoint end(XYZCoord(500, 500, 0), HALF_PI);
    rrt.fillOptions(0, {end});

    // all four of the CSC paths exist between two vectors this far apart
    EXPECT_EQ(rrt.options.size(), 4);
    for (const PathSegment& option : rrt.options) {
        EXPECT_TRUE(option.end == end);
        EXPECT_TRUE(std::isfinite(option.path.length));
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
 *  RRT::bestConnection -- a path that leaves the airspace is not one that can be
 *  taken, no matter how cheap it is
 */
TEST(RRTTest, BestConnectionStaysInsideTheAirspace) {
    initFieldWithWall();
    RRT rrt(RRTPoint(XYZCoord(200, 400, 0), 0));

    // the plane is boxed in against the wall, so the way to some of these is not
    // the shortest one
    const std::vector<RRTPoint> targets = {RRTPoint(XYZCoord(400, 200, 0), M_PI),
                                           RRTPoint(XYZCoord(300, 800, 0), HALF_PI),
                                           RRTPoint(XYZCoord(120, 400, 0), M_PI),
                                           RRTPoint(XYZCoord(460, 650, 0), 0)};

    for (const RRTPoint& end : targets) {
        const auto [anchor, segment] = rrt.bestConnection({end});
        const double cost = (anchor == INVALID_NODE) ? std::numeric_limits<double>::infinity()
                                                     : costOf(rrt, anchor, segment);

        if (anchor != INVALID_NODE) {
            EXPECT_TRUE(Environment::isDubinsPathInBounds(rrt.tree.point(anchor), segment.end,
                                                          segment.path));
        }

        // everything cheaper than what it settled on cuts through the wall or the
        // edge of the field. These options come off the root, which the tree
        // starts at, so the path length is the whole cost of the flight
        rrt.fillOptions(0, {end});
        ASSERT_FALSE(rrt.options.empty());

        for (const PathSegment& option : rrt.options) {
            if (option.path.length < cost) {
                EXPECT_FALSE(Environment::isDubinsPathInBounds(rrt.tree.point(0), option.end,
                                                               option.path))
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
    RRT rrt(RRTPoint(XYZCoord(100, 100, 0), 0));

    // a couple of branches, so the nodes sit at different distances from the root
    const RRTPoint near_node(XYZCoord(200, 150, 0), 0);
    const RRTPoint far_node(XYZCoord(700, 200, 0), HALF_PI);
    rrt.tree.addSample(0, {near_node, Dubins::bestOption(rrt.tree.getStart(), near_node)});
    rrt.tree.addSample(1, {far_node, Dubins::bestOption(near_node, far_node)});
    ASSERT_EQ(rrt.tree.size, 3);

    const std::vector<RRTPoint> ends = {RRTPoint(XYZCoord(800, 800, 0), HALF_PI)};
    const auto [anchor, segment] = rrt.bestConnection(ends);
    const auto [cheapest_anchor, cheapest_cost] = cheapestFlyableConnection(rrt, ends);

    ASSERT_NE(anchor, INVALID_NODE);
    EXPECT_EQ(anchor, cheapest_anchor);
    EXPECT_DOUBLE_EQ(costOf(rrt, anchor, segment), cheapest_cost);
    EXPECT_TRUE(segment.end == ends[0]);

    // the tree is left alone -- the caller decides whether to commit
    EXPECT_EQ(rrt.tree.size, 3);
}

/*
 *  RRT::bestConnection -- any of the points will do, and the cheapest one wins
 */
TEST(RRTTest, BestConnectionTakesTheCheapestOfTheEndpoints) {
    initOpenField();
    RRT rrt(RRTPoint(XYZCoord(100, 100, 0), 0));

    // straight ahead of the plane, and well off to the side of it
    const RRTPoint close(XYZCoord(300, 100, 0), 0);
    const RRTPoint distant(XYZCoord(800, 700, 0), M_PI);

    const auto [anchor, segment] = rrt.bestConnection({distant, close});

    ASSERT_NE(anchor, INVALID_NODE);
    EXPECT_TRUE(segment.end == close);
    EXPECT_DOUBLE_EQ(costOf(rrt, anchor, segment),
                     cheapestFlyableConnection(rrt, {distant, close}).second);
}

/*
 *  RRT::bestConnection -- an unreachable point hands back an invalid connection
 */
TEST(RRTTest, BestConnectionGivesUpOnAnUnreachablePoint) {
    initOpenField();
    RRT rrt(RRTPoint(XYZCoord(100, 100, 0), 0));

    EXPECT_EQ(rrt.bestConnection({RRTPoint(XYZCoord(2000, 2000, 0), 0)}).first, INVALID_NODE);
}

/*
 *  RRT::bestConnection -- a point the wall stands in front of is given up on, while
 *  one on the same side of it as the plane is still found
 */
TEST(RRTTest, BestConnectionGivesUpOnAPointBehindAnObstacle) {
    initFieldWithWall();
    RRT rrt(RRTPoint(XYZCoord(200, 400, 0), 0));

    // a single node cannot reach around the wall -- the gap is 300m above it, and
    // every path that lands on this point comes in through the wall
    const RRTPoint across(XYZCoord(800, 400, 0), 0);
    EXPECT_EQ(rrt.bestConnection({across}).first, INVALID_NODE);

    // the same tree still finds a point the wall is not in front of
    const RRTPoint reachable(XYZCoord(400, 200, 0), M_PI);
    EXPECT_NE(rrt.bestConnection({reachable}).first, INVALID_NODE);

    // and a point behind the wall does not stop the reachable one from winning
    const auto [anchor, segment] = rrt.bestConnection({across, reachable});
    ASSERT_NE(anchor, INVALID_NODE);
    EXPECT_TRUE(segment.end == reachable);
}

/*
 *  RRT::connectToGoal -- the leg is handed back, and where the tree is rooted is
 *  left for the caller to decide
 */
TEST(RRTTest, ConnectToGoalCommitsThePathWithoutMovingTheRoot) {
    initOpenField();
    const RRTPoint start(XYZCoord(100, 100, 0), 0);
    const XYZCoord goal(500, 500, 30);
    RRT rrt(start);

    const Leg leg = rrt.connectToGoal(goalEndpoints(goal, DEFAULT_GOAL_ANGLES));

    ASSERT_TRUE(leg.isValid());
    EXPECT_TRUE(leg.start == start);
    EXPECT_TRUE(leg.end.coord == goal);
    EXPECT_GT(leg.length, 0);

    const std::vector<XYZCoord> path = flyLeg(leg);
    ASSERT_FALSE(path.empty());
    EXPECT_TRUE(pathIsInBounds(path));
    EXPECT_NEAR(path.back().x, goal.x, 1e-6);
    EXPECT_NEAR(path.back().y, goal.y, 1e-6);

    EXPECT_NE(std::find(DEFAULT_GOAL_ANGLES.begin(), DEFAULT_GOAL_ANGLES.end(), leg.end.psi),
              DEFAULT_GOAL_ANGLES.end());

    // the goal was hung off the tree, but the tree is still rooted where the
    // plane is -- the leg has been priced, not taken
    EXPECT_TRUE(rrt.tree.getStart() == start);
    EXPECT_EQ(rrt.tree.size, 2);

    // taking it is what makes the waypoint the root the next leg is flown from
    rrt.reroot(leg.end);
    EXPECT_EQ(rrt.tree.size, 1);
    EXPECT_TRUE(rrt.tree.getStart() == flat(leg.end));
}

/*
 *  RRT::connectToGoal -- a goal outside the airspace is never connected to
 */
TEST(RRTTest, ConnectToGoalFailsOnAnUnreachableGoal) {
    initOpenField();
    const RRTPoint start(XYZCoord(100, 100, 0), 0);
    RRT rrt(start);

    const Leg leg = rrt.connectToGoal(goalEndpoints(XYZCoord(2000, 2000, 30), DEFAULT_GOAL_ANGLES));

    EXPECT_FALSE(leg.isValid());

    // the tree is left as it was, still rooted at the plane
    EXPECT_EQ(rrt.tree.size, 1);
    EXPECT_TRUE(rrt.tree.getStart() == start);
}

/*
 *  RRT::RRTIteration -- the goal is connected to once the sampling is done
 */
TEST(RRTTest, RRTIterationConnectsToAReachableGoal) {
    initOpenField();
    seedRandom(11);
    const XYZCoord goal(700, 700, 30);
    RRT rrt(RRTPoint(XYZCoord(100, 100, 0), 0));

    const Leg leg = rrt.RRTIteration(goal, goalEndpoints(goal, DEFAULT_GOAL_ANGLES));

    ASSERT_TRUE(leg.isValid());
    EXPECT_TRUE(leg.end.coord == goal);
    EXPECT_TRUE(pathIsInBounds(flyLeg(leg)));

    // sampling grew the tree, but the leg is still flown from where the plane is
    EXPECT_TRUE(leg.start == rrt.tree.getStart());
    EXPECT_TRUE(rrt.tree.getStart().coord == XYZCoord(100, 100, 0));
}

/*
 *  RRT::run -- the leg found around an obstacle stays in bounds the whole way
 */
TEST(RRTTest, RunPathsAroundAnObstacle) {
    initFieldWithWall();
    seedRandom(23);
    const XYZCoord goal(800, 400, 30);
    RRT rrt(RRTPoint(XYZCoord(200, 400, 0), 0));

    const Leg leg = rrt.run(goal, DEFAULT_GOAL_ANGLES);

    ASSERT_TRUE(leg.isValid()) << "never made it around the wall";
    EXPECT_TRUE(leg.end.coord == goal);

    const std::vector<XYZCoord> path = flyLeg(leg);
    ASSERT_FALSE(path.empty());
    EXPECT_TRUE(pathIsInBounds(path));

    // a straightaway is described by its two endpoints alone, so checking the
    // points is not enough -- no leg of the path may cut through the wall
    for (std::size_t i = 1; i < path.size(); i++) {
        EXPECT_FALSE(Environment::doesLineIntersectPolygon(path[i - 1], path[i], WALL))
            << "leg " << i << " cuts through the wall";
    }
}

/*
 *  RRT::run -- a goal that can be flown to directly is, without ever sampling
 */
TEST(RRTTest, RunTakesTheDirectFlightWhenThereIsOne) {
    initOpenField();
    const RRTPoint start(XYZCoord(100, 100, 0), 0);
    RRT rrt(start);

    const XYZCoord goal(500, 500, 30);
    const Leg leg = rrt.run(goal, DEFAULT_GOAL_ANGLES);

    ASSERT_TRUE(leg.isValid());
    EXPECT_TRUE(leg.start == start);

    // straight off the root, so the leg is the one dubins path that got there
    EXPECT_EQ(leg.segments.size(), 1);
    EXPECT_DOUBLE_EQ(leg.length, leg.segments.back().path.length);
}

/*
 *  RRT::run -- a run prices a leg without taking it, so two goals can be weighed
 *  from the same place. Rerooting is what flies one of them.
 */
TEST(RRTTest, RunLeavesTheRootForTheCallerToMove) {
    initOpenField();
    seedRandom(5);
    const RRTPoint start(XYZCoord(100, 100, 0), 0);
    RRT rrt(start);

    // both legs are flown from where the plane is, not from each other
    const Leg first = rrt.run(XYZCoord(400, 300, 30), DEFAULT_GOAL_ANGLES);
    const Leg alternative = rrt.run(XYZCoord(800, 600, 45), DEFAULT_GOAL_ANGLES);

    ASSERT_TRUE(first.isValid());
    ASSERT_TRUE(alternative.isValid());
    EXPECT_TRUE(first.start == start);
    EXPECT_TRUE(alternative.start == start);
    EXPECT_TRUE(rrt.tree.getStart() == start);

    // taking one of them is what moves the plane on
    rrt.reroot(first.end);
    const Leg second = rrt.run(XYZCoord(800, 600, 45), DEFAULT_GOAL_ANGLES);

    ASSERT_TRUE(second.isValid());
    EXPECT_TRUE(second.start == flat(first.end));
    EXPECT_TRUE(rrt.tree.getStart() == flat(first.end));
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
        RRT rrt(RRTPoint(start, 0));

        // a tree of random samples, grown the way an iteration would grow it
        for (int i = 0; i < 40; i++) {
            const RRTPoint sample(Environment::getRandomPoint(false, start), random(0, TWO_PI));
            const auto [anchor, segment] = rrt.bestConnection({sample});

            if (anchor != INVALID_NODE) {
                rrt.tree.addSample(anchor, segment);
            }
        }
        ASSERT_GT(rrt.tree.size, 1) << "trial " << trial << " grew nothing to search";

        const std::vector<RRTPoint> ends = goalEndpoints(goal, DEFAULT_GOAL_ANGLES);
        const auto [found, found_segment] = rrt.bestConnection(ends);
        const auto [exhaustive, exhaustive_cost] = cheapestFlyableConnection(rrt, ends);

        ASSERT_EQ(found == INVALID_NODE, exhaustive == INVALID_NODE) << "trial " << trial;

        if (found != INVALID_NODE) {
            EXPECT_DOUBLE_EQ(costOf(rrt, found, found_segment), exhaustive_cost)
                << "trial " << trial;
            EXPECT_EQ(found, exhaustive) << "trial " << trial;
        }
    }
}
