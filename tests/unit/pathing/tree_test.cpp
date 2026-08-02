#include "pathing/tree.hpp"

#include <gtest/gtest.h>

#include <vector>

#include "pathing/dubins.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

namespace {

static inline void setDubins(double r, double sep) {
    Dubins::_radius = r;
    Dubins::_point_separation = sep;
}

// the tree only ever reads a dubins option's length, the rest is carried untouched
RRTOption option(double length) { return RRTOption(length, DubinsPath(0, 0, length), true); }

RRTPoint point(double x, double y) { return RRTPoint(XYZCoord(x, y, 0), 0); }

// the children of a node, in the order the sibling list holds them
std::vector<NodeId> childrenOf(const RRTTree& tree, NodeId parent) {
    std::vector<NodeId> children;
    for (NodeId child = tree.tree.first_child[parent]; child != INVALID_NODE;
         child = tree.tree.next_sibling[child]) {
        children.push_back(child);
    }
    return children;
}

std::vector<double> lengthsOf(const std::vector<PathSegment>& segments) {
    std::vector<double> lengths;
    for (const PathSegment& segment : segments) {
        lengths.push_back(segment.option.length);
    }
    return lengths;
}

}  // namespace

/*
 *  The root is the only node a fresh tree has, and it is node 0
 */
TEST(TreeTest, RootIsTheOnlyNodeAtConstruction) {
    const RRTPoint root_point = point(25, 25);
    RRTTree tree(root_point);

    EXPECT_EQ(tree.tree.size, 1);
    EXPECT_TRUE(tree.getStart() == root_point);
    EXPECT_TRUE(tree.tree.points[0] == root_point);

    EXPECT_EQ(tree.tree.parent[0], INVALID_NODE);
    EXPECT_EQ(tree.tree.first_child[0], INVALID_NODE);
    EXPECT_EQ(tree.tree.next_sibling[0], INVALID_NODE);
    EXPECT_EQ(tree.tree.length[0], 0.0);

    // the root's option is a no-op, the path starts where the root is
    EXPECT_EQ(tree.tree.rrt_options[0].length, 0.0);
}

/*
 *  The tree is fixed size, and big enough to hold every iteration RRT will run
 */
TEST(TreeTest, CapacityCoversEveryIteration) {
    RRTTree tree(point(0, 0));

    // the root, every sample of an iteration, and the goal they connect to
    EXPECT_EQ(ITERATIONS_PER_WAYPOINT, TREE_CAPACITY - 2);

    EXPECT_EQ(tree.tree.points.size(), TREE_CAPACITY);
    EXPECT_EQ(tree.tree.rrt_options.size(), TREE_CAPACITY);
    EXPECT_EQ(tree.tree.length.size(), TREE_CAPACITY);
    EXPECT_EQ(tree.tree.parent.size(), TREE_CAPACITY);
    EXPECT_EQ(tree.tree.first_child.size(), TREE_CAPACITY);
    EXPECT_EQ(tree.tree.next_sibling.size(), TREE_CAPACITY);
}

/*
 *  A sample hangs off its parent, and starts out as a childless leaf
 */
TEST(TreeTest, AddSampleLinksToItsParent) {
    RRTTree tree(point(0, 0));
    const RRTPoint sample = point(10, 10);

    tree.addSample(0, sample, option(14.0));

    ASSERT_EQ(tree.tree.size, 2);
    EXPECT_TRUE(tree.tree.points[1] == sample);
    EXPECT_EQ(tree.tree.parent[1], 0);
    EXPECT_EQ(tree.tree.first_child[1], INVALID_NODE);
    EXPECT_EQ(tree.tree.next_sibling[1], INVALID_NODE);
    EXPECT_EQ(tree.tree.rrt_options[1].length, 14.0);

    // the root now points at it
    EXPECT_EQ(tree.tree.first_child[0], 1);
    EXPECT_EQ(childrenOf(tree, 0), std::vector<NodeId>({1}));
}

/*
 *  Slots are handed out in order -- RRT relies on this to know the id a sample
 *  will land at before it is added
 */
TEST(TreeTest, NodeIdsAreHandedOutInOrder) {
    RRTTree tree(point(0, 0));

    for (NodeId expected = 1; expected < 10; expected++) {
        EXPECT_EQ(tree.tree.size, expected);
        tree.addSample(0, point(expected, 0), option(1.0));
        EXPECT_TRUE(tree.tree.points[expected] == point(expected, 0));
    }
}

/*
 *  Siblings are appended to the end of the list, so children stay in insertion order
 */
TEST(TreeTest, ChildrenKeepInsertionOrder) {
    RRTTree tree(point(0, 0));

    tree.addSample(0, point(1, 0), option(1.0));
    tree.addSample(0, point(2, 0), option(2.0));
    tree.addSample(0, point(3, 0), option(3.0));

    EXPECT_EQ(childrenOf(tree, 0), std::vector<NodeId>({1, 2, 3}));

    // every child knows the root as its parent, and none of them have children
    for (const NodeId child : childrenOf(tree, 0)) {
        EXPECT_EQ(tree.tree.parent[child], 0);
        EXPECT_EQ(tree.tree.first_child[child], INVALID_NODE);
    }

    // a child of a child does not end up in the root's list
    tree.addSample(2, point(2, 1), option(1.0));
    EXPECT_EQ(childrenOf(tree, 0), std::vector<NodeId>({1, 2, 3}));
    EXPECT_EQ(childrenOf(tree, 2), std::vector<NodeId>({4}));
}

/*
 *  A node's length is the distance flown from the root to reach it
 */
TEST(TreeTest, LengthAccumulatesDownABranch) {
    RRTTree tree(point(0, 0));

    tree.addSample(0, point(1, 0), option(10.0));  // node 1
    tree.addSample(1, point(2, 0), option(2.5));   // node 2
    tree.addSample(2, point(3, 0), option(7.5));   // node 3

    // a second branch off of the root, to make sure lengths are not shared
    tree.addSample(0, point(0, 1), option(100.0));  // node 4

    EXPECT_DOUBLE_EQ(tree.tree.length[1], 10.0);
    EXPECT_DOUBLE_EQ(tree.tree.length[2], 12.5);
    EXPECT_DOUBLE_EQ(tree.tree.length[3], 20.0);
    EXPECT_DOUBLE_EQ(tree.tree.length[4], 100.0);
}

/*
 *  RRTTree::findPathToNode -- the options are returned in flight order
 */
TEST(TreeTest, FindPathToNodeReturnsOptionsInFlightOrder) {
    RRTTree tree(point(0, 0));

    tree.addSample(0, point(1, 0), option(10.0));  // node 1
    tree.addSample(1, point(2, 0), option(20.0));  // node 2
    tree.addSample(2, point(3, 0), option(30.0));  // node 3

    EXPECT_EQ(lengthsOf(tree.findPathToNode(3)), std::vector<double>({10.0, 20.0, 30.0}));
    EXPECT_EQ(lengthsOf(tree.findPathToNode(2)), std::vector<double>({10.0, 20.0}));
    EXPECT_EQ(lengthsOf(tree.findPathToNode(1)), std::vector<double>({10.0}));
}

/*
 *  RRTTree::findPathToNode -- every segment carries the vector its option lands
 *  on, which is the point stored on the node the option belongs to
 */
TEST(TreeTest, FindPathToNodeCarriesTheEndPoints) {
    RRTTree tree(point(0, 0));

    tree.addSample(0, point(1, 0), option(10.0));  // node 1
    tree.addSample(1, point(2, 0), option(20.0));  // node 2
    tree.addSample(2, point(3, 0), option(30.0));  // node 3

    const std::vector<PathSegment> segments = tree.findPathToNode(3);

    ASSERT_EQ(segments.size(), 3);
    for (NodeId node = 1; node <= 3; node++) {
        EXPECT_TRUE(segments[node - 1].end == tree.tree.points[node]);
    }
}

/*
 *  RRTTree::findPathToNode -- only the target's own ancestors are on the path
 */
TEST(TreeTest, FindPathToNodeOnlyWalksAncestors) {
    RRTTree tree(point(0, 0));

    tree.addSample(0, point(1, 0), option(10.0));  // node 1, on the path
    tree.addSample(0, point(0, 1), option(50.0));  // node 2, a sibling branch
    tree.addSample(2, point(0, 2), option(60.0));  // node 3, hangs off the sibling
    tree.addSample(1, point(2, 0), option(20.0));  // node 4, the target

    EXPECT_EQ(lengthsOf(tree.findPathToNode(4)), std::vector<double>({10.0, 20.0}));
    EXPECT_EQ(lengthsOf(tree.findPathToNode(3)), std::vector<double>({50.0, 60.0}));
}

/*
 *  RRTTree::findPathToNode -- the root is where the path starts, so it flies nothing
 */
TEST(TreeTest, FindPathToNodeHandlesRootAndInvalidNode) {
    RRTTree tree(point(0, 0));
    tree.addSample(0, point(1, 0), option(10.0));

    EXPECT_TRUE(tree.findPathToNode(0).empty());
    EXPECT_TRUE(tree.findPathToNode(INVALID_NODE).empty());
}

/*
 *  RRTTree::setCurrentHead -- the tree is thrown away and restarted at the new head
 */
TEST(TreeTest, SetCurrentHeadRestartsTheTree) {
    RRTTree tree(point(0, 0));

    tree.addSample(0, point(1, 0), option(10.0));
    tree.addSample(1, point(2, 0), option(20.0));
    tree.addSample(0, point(0, 1), option(30.0));
    ASSERT_EQ(tree.tree.size, 4);

    const RRTPoint new_head(XYZCoord(50, 50, 0), M_PI);
    tree.setCurrentHead(new_head);

    EXPECT_EQ(tree.tree.size, 1);
    EXPECT_TRUE(tree.getStart() == new_head);
    EXPECT_EQ(tree.tree.parent[0], INVALID_NODE);
    EXPECT_EQ(tree.tree.first_child[0], INVALID_NODE);
    EXPECT_EQ(tree.tree.next_sibling[0], INVALID_NODE);
    EXPECT_EQ(tree.tree.length[0], 0.0);
    EXPECT_TRUE(tree.findPathToNode(0).empty());

    // the slots the old tree used are handed back out
    tree.addSample(0, point(51, 50), option(1.0));
    EXPECT_EQ(tree.tree.size, 2);
    EXPECT_EQ(tree.tree.parent[1], 0);
    EXPECT_DOUBLE_EQ(tree.tree.length[1], 1.0);
    EXPECT_EQ(childrenOf(tree, 0), std::vector<NodeId>({1}));
}

/*
 *  The tree fills up to its capacity, one node per sample
 */
TEST(TreeTest, FillsToCapacity) {
    RRTTree tree(point(0, 0));

    // the root already took a slot
    for (NodeId i = 1; i < ITERATIONS_PER_WAYPOINT; i++) {
        tree.addSample(i - 1, point(i, 0), option(1.0));
    }

    EXPECT_EQ(tree.tree.size, ITERATIONS_PER_WAYPOINT);
    EXPECT_DOUBLE_EQ(tree.tree.length[ITERATIONS_PER_WAYPOINT - 1], ITERATIONS_PER_WAYPOINT - 1);
    EXPECT_EQ(tree.findPathToNode(ITERATIONS_PER_WAYPOINT - 1).size(),
              ITERATIONS_PER_WAYPOINT - 1);
}

/*
 *  The path findPathToNode hands back, flown from the head, actually arrives at
 *  the node it was asked for
 */
TEST(TreeTest, PathToNodeFlownFromHeadReachesTheNode) {
    setDubins(5, 1);

    const RRTPoint root_point(XYZCoord(0, 0, 0), 0);
    RRTTree tree(root_point);

    const std::vector<RRTPoint> samples = {
        RRTPoint(XYZCoord(30, 10, 0), M_PI / 4),
        RRTPoint(XYZCoord(60, 40, 0), M_PI / 2),
        RRTPoint(XYZCoord(20, 70, 0), M_PI),
    };

    NodeId parent = 0;
    for (const RRTPoint& sample : samples) {
        tree.addSample(parent, sample, Dubins::bestOption(tree.tree.points[parent], sample));
        parent = tree.tree.size - 1;
    }

    const std::vector<XYZCoord> path =
        Dubins::generatePath(tree.getStart(), tree.findPathToNode(parent));

    ASSERT_FALSE(path.empty());
    EXPECT_NEAR(path.back().x, samples.back().coord.x, 0.01);
    EXPECT_NEAR(path.back().y, samples.back().coord.y, 0.01);

    // the length stored on the node is the length of that path
    double flown = tree.getStart().coord.distanceTo(path[0]);
    for (std::size_t i = 1; i < path.size(); i++) {
        flown += path[i - 1].distanceTo(path[i]);
    }
    // the generated points cut corners off of every arc, so the polyline is a
    // little shorter than the arc length the tree tracks
    EXPECT_LT(flown, tree.tree.length[parent] + 0.01);
    EXPECT_GT(flown, tree.tree.length[parent] * 0.98);
}
