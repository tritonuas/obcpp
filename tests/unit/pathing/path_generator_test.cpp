#include "pathing/path_generator.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <vector>

#include "field.hpp"
#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "pathing/rrt.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

/*
 *  A fresh generator holds nothing but the plane's current vector, which is the
 *  first of the points it flies through
 */
TEST(PathGeneratorTest, ConstructionSeedsTheTreeWithTheStart) {
    initOpenField();
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(500, 500, 30)};

    const PathGenerator generator(goals, HALF_PI);

    // the search is rooted where the plane is, flying the heading it was given
    EXPECT_TRUE(generator.rrt.tree.getStart() == RRTPoint(goals[0], HALF_PI));
    EXPECT_EQ(generator.rrt.tree.size, 1);
    EXPECT_TRUE(generator.getPointsToGoal().empty());
    EXPECT_EQ(generator.goals, goals);

    // every approach angle is tried at every goal unless the caller asks for a
    // specific set
    ASSERT_EQ(generator.goal_angles.size(), goals.size());
    EXPECT_EQ(generator.goal_angles[1], DEFAULT_GOAL_ANGLES);

    const std::vector<double> angles = {0.0, M_PI};
    const PathGenerator custom_angles(goals, HALF_PI, angles);
    EXPECT_EQ(custom_angles.goal_angles[1], angles);
}

/*
 *  A caller that dictates how a goal is approached -- coverage pathing does, as a
 *  scan line has to be flown along its own direction -- leaves it a single angle
 */
TEST(PathGeneratorTest, ConstructionPinsTheGoalsTheCallerNamedAnAngleFor) {
    initOpenField();
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(500, 500, 30),
                                         XYZCoord(800, 200, 30)};
    const std::vector<std::vector<double>> goal_angles = {{}, {0}, {M_PI}};

    const PathGenerator generator(goals, HALF_PI, goal_angles);

    EXPECT_TRUE(generator.rrt.tree.getStart() == RRTPoint(goals[0], HALF_PI));
    EXPECT_EQ(generator.rrt.tree.size, 1);
    EXPECT_EQ(generator.goals, goals);

    // the goal the plane is already sitting on is never approached, so whatever
    // the caller put down for it is left alone and never read
    EXPECT_EQ(generator.goal_angles[1], std::vector<double>({0}));
    EXPECT_EQ(generator.goal_angles[2], std::vector<double>({M_PI}));
}

/*
 *  PathGenerator::run -- the way between the waypoints is up to RRT, but the
 *  heading a pinned one is reached at is not
 */
TEST(PathGeneratorTest, RunReachesEveryWaypointAtItsPinnedAngle) {
    initOpenField();
    seedRandom(7);

    // scan lines, the way coverage pathing lays them out: swept one way, then back
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(200, 300, 30),
                                         XYZCoord(800, 300, 30), XYZCoord(800, 400, 30),
                                         XYZCoord(200, 400, 30)};
    const std::vector<std::vector<double>> goal_angles = {{}, {0}, {0}, {M_PI}, {M_PI}};

    PathGenerator generator(goals, 0, goal_angles);
    generator.run();

    const std::vector<XYZCoord> path = generator.getPointsToGoal();
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
 *  PathGenerator::generateDubinsOptions finds the paths and stops there -- what
 *  they cost is known without flying them, so a mission can be weighed against
 *  another one and thrown away without ever generating a point
 */
TEST(PathGeneratorTest, DubinsOptionsAreFoundWithoutFlyingThem) {
    initOpenField();
    seedRandom(13);
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(400, 300, 30),
                                         XYZCoord(800, 600, 30)};

    PathGenerator generator(goals, 0);
    EXPECT_EQ(generator.pathLength(), 0);
    EXPECT_TRUE(generator.getPointsToGoal().empty());

    generator.generateDubinsOptions();

    // one leg per waypoint flown to, each landing on the goal it was found for.
    // RRT ignores altitude, so the paths out of the tree are flat
    ASSERT_EQ(generator.legs.size(), goals.size() - 1);
    for (std::size_t i = 0; i < generator.legs.size(); i++) {
        EXPECT_TRUE(generator.legs[i].end.coord == goals[i + 1]);
        EXPECT_FALSE(generator.legs[i].segments.empty());
        EXPECT_TRUE(generator.legs[i].segments.back().end == flat(generator.legs[i].end));
        EXPECT_GT(generator.legs[i].length, 0);
    }

    // the legs start where the one behind them landed
    EXPECT_TRUE(generator.legs[0].start == RRTPoint(goals[0], 0));
    EXPECT_TRUE(generator.legs[1].start == flat(generator.legs[0].end));

    // how long the mission is is known, but not one point of it has been flown
    double straight_line = 0;
    for (std::size_t i = 1; i < goals.size(); i++) {
        straight_line += std::hypot(goals[i].x - goals[i - 1].x, goals[i].y - goals[i - 1].y);
    }
    EXPECT_GE(generator.pathLength(), straight_line);
    EXPECT_TRUE(generator.getPointsToGoal().empty());

    generator.generateFlightPoints();

    const std::vector<XYZCoord> path = generator.getPointsToGoal();
    ASSERT_FALSE(path.empty());

    double flown = goals[0].distanceTo(path[0]);
    for (std::size_t i = 1; i < path.size(); i++) {
        flown += std::hypot(path[i].x - path[i - 1].x, path[i].y - path[i - 1].y);
    }

    // the points cut the corners off the arcs, so they cover a little less ground
    // than the legs they came from
    EXPECT_LT(flown, generator.pathLength());
    EXPECT_GT(flown, generator.pathLength() * 0.95);

    // flying the legs a second time does not append the mission to itself
    generator.generateFlightPoints();
    EXPECT_EQ(generator.getPointsToGoal().size(), path.size());

    // and run() is the two of them, one after the other
    PathGenerator ran(goals, 0);
    seedRandom(13);
    ran.run();
    EXPECT_EQ(ran.legs.size(), generator.legs.size());
    EXPECT_EQ(ran.getPointsToGoal().size(), path.size());
}

/*
 *  PathGenerator::buildFlightPath -- the plane climbs at a constant rate along the
 *  ground it covers, so the altitude is interpolated over the length of the leg
 */
TEST(PathGeneratorTest, FlightPointsClimbToTheWaypointAltitude) {
    initOpenField();
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(500, 500, 100),
                                         XYZCoord(800, 200, 50)};
    PathGenerator generator(goals, 0);

    // first leg: climbs from the plane's altitude (0) up to 100
    const RRTPoint first_goal(goals[1], 0);
    generator.legs.push_back(generator.rrt.commitConnection(
        0, {first_goal, Dubins::bestOption(generator.rrt.tree.getStart(), first_goal)}));
    generator.rrt.reroot(first_goal);
    generator.generateFlightPoints();

    const std::vector<XYZCoord> first_leg = generator.getPointsToGoal();
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
    generator.legs.push_back(generator.rrt.commitConnection(
        0, {second_goal, Dubins::bestOption(generator.rrt.tree.getStart(), second_goal)}));
    generator.rrt.reroot(second_goal);
    generator.generateFlightPoints();

    const std::vector<XYZCoord> path = generator.getPointsToGoal();
    ASSERT_GT(path.size(), first_leg.size());
    EXPECT_NEAR(path.back().z, 50, 1e-9);

    for (std::size_t i = first_leg.size(); i < path.size(); i++) {
        EXPECT_LE(path[i].z, 100);
        EXPECT_GE(path[i].z, 50);
        EXPECT_LE(path[i].z, path[i - 1].z);
    }
}

/*
 *  PathGenerator::run -- every waypoint is flown, in order
 */
TEST(PathGeneratorTest, RunFliesEveryWaypointInOrder) {
    initOpenField();
    seedRandom(3);
    const std::vector<XYZCoord> goals = {XYZCoord(100, 100, 0), XYZCoord(400, 300, 30),
                                         XYZCoord(800, 600, 45), XYZCoord(200, 800, 60)};
    PathGenerator generator(goals, 0);

    generator.run();

    const std::vector<XYZCoord> path = generator.getPointsToGoal();
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

    // the path ends on the last waypoint, and so does the search
    EXPECT_EQ(indexOfPoint(path, goals.back(), index), path.size() - 1);
    EXPECT_TRUE(generator.rrt.tree.getStart().coord == flat(goals.back()));
}

/*
 *  PathGenerator::run -- the path found around an obstacle stays in bounds the
 *  whole way
 */
TEST(PathGeneratorTest, RunPathsAroundAnObstacle) {
    initFieldWithWall();
    seedRandom(23);
    const XYZCoord goal(800, 400, 30);
    PathGenerator generator({XYZCoord(200, 400, 0), goal}, 0);

    generator.run();

    const std::vector<XYZCoord> path = generator.getPointsToGoal();
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
