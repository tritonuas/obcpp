#ifndef PATHING_STATIC_HPP_
#define PATHING_STATIC_HPP_

#include <cmath>
#include <utility>
#include <vector>

#include "core/states.hpp"
#include "pathing/dubins.hpp"
#include "pathing/tree.hpp"

enum Validity
{
    VALID,
    INVALID,
    GOAL
};

std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state)
{
    // do pathing here
    return {};
}

/**
 * Generates a random_01 number between 0 and 1
 *
 * @return  ==> double [0,1]
 */
double random_01()
{
    return static_cast<double>(std::rand()) / RAND_MAX;
}

class RRT
{
public:
    const double DISTANCE_GUESSED = 10; // the radius that the point will generate from center, need to change to be a better system.

    RRT(RRTPoint start,
        RRTPoint goal,
        int num_iterations,
        double goal_bias,
        double search_radius,
        double tolerance_to_goal,
        double rewire_radius,
        Polygon bounds)

        : _root{&RRTNode{start, 0}},
          _goal{goal},
          _num_iterations{num_iterations},
          _goal_bias{goal_bias},
          _search_radius{search_radius},
          _tolerance_to_goal{tolerance_to_goal},
          _rewire_radius{rewire_radius},
          _dubins{TURNING_RADIUS, POINT_SEPARATION},
          _found_goal{false},
          _goal_node{nullptr},
          _bounds{bounds}
    {
        _tree.addNode(nullptr, _root, {}, 0);
    }

    // choose a random point from free space, not nessisarily this
    RRTPoint generateSamplePoint(const RRTPoint &from)
    {
        if (random_01() < _goal_bias)
        {
            return RRTPoint{_goal.point, random_01() * TWO_PI};
        }

        // TODO - get point from free space
    }

    /**
     * checks if the generated path of a dubins curve is valid in the flight
     * zonw.
     *
     * @param path  ==> list of points in a given flight path
     * @return      ==> whether or not every point is within the bounds of the
     *                  area
     */
    bool validPath(const std::vector<XYZCoord> &path)
    {
        // polygons dont work yet? also should the be convex?
        for (const XYZCoord &point : path)
        {
            // [TODO], add if the point is B] not inside an obsticle C] can turn
            //          around boundaries
            if (!_bounds.pointInBounds(point))
            {
                return false;
            }
        }

        return true;
    }

    /**
     * RRT*
     */
    void run()
    {
        _distance_to_goal = _root->getPoint().distanceTo(_goal);

        for (int _; _ < _num_iterations; _++)
        {
            if (_found_goal)
            {
                return;
            }

            // NEEDS TO CHANGE but what lol (these two comments were written by the same person)
            RRTPoint sample = generateSamplePoint(_root->getPoint());

            std::vector<std::pair<RRTNode *, RRTOption>> options = pathingOptions(sample);

            // [TODO] add rest of function
            Validity validity = parseOptions(options, sample);

            switch (validity)
            {
            case INVALID:
                continue; // test propertly
            case GOAL:
                _found_goal = true;
                break;
            case VALID:
                break; // test properly
            }
        }
    }

    /**
     * ?????????
     */
    Validity parseOptions(std::vector<std::pair<RRTNode *, RRTOption>> &options, const RRTPoint &sample)
    {
        for (const auto &[node, option] : options)
        {
            if (option.length == std::numeric_limits<double>::infinity())
            {
                return INVALID;
            }

            std::vector<XYZCoord> path = _dubins.generatePoints(_root->getPoint(), sample, option.dubins_path, option.has_straight);

            // TODO: check whats in bounds or not
            if (!validPath(path))
            {
                continue;
            }

            // if the node is the sample, return (preventes loops)
            if (node->getPoint() == sample)
            {
                return INVALID;
            }

            RRTNode new_node{sample, node->getCost() + option.length};

            // If node is uncompetitive, discard it
            if (_tree.getNode(sample) != nullptr && new_node.getCost() >= _tree.getNode(sample)->getCost())
            {
                continue;
            }

            // else, add the node to the
            _tree.addNode(node, &new_node, _dubins.generatePoints(node->getPoint(), sample, option.dubins_path, option.has_straight), new_node.getCost());

            // for clarity, sample is not used beyond this point. It is now the new_node
            // TODO, call optimize_tree
            optimizeTree(new_node);

            // if the new node is within the tolerance of the goal, return ???
            // TODO
            if (inGoalRegion(new_node.getPoint()))
            {
                _found_goal = true;
                _goal_node = &new_node;
                return GOAL;
            }

            return VALID;
        }
    }

    /**
     * Returns a sorted list of the paths to get from a given node to the sampled
     * node
     *
     * @param end               ==> the sampled node that needs to be connected
     *                              to the tree
     * @param quantity_options  ==> the number of results to return back to the
     *                              function
     * @return                  ==> mininum sorted list of pairs of <node, path>
     */
    std::vector<std::pair<RRTNode *, RRTOption>> pathingOptions(const RRTPoint &end, int quantity_options = 10)
    {
        std::vector<std::pair<RRTNode *, RRTOption>> options;

        fillOptions(options, _root);

        if (options.size() < quantity_options)
        {
            return options;
        }

        // TODO, learn how to write a lambda function
        std::sort(options.begin(), options.end(), [](auto a, auto b)
                  { return compareRRTOptionLength(a.second, b.second) });

        return {options.begin(), options.begin() + quantity_options};
    }

    /**
     * traverses the tree, and puts in all RRTOptions from dubins into a list
     * (DFS)
     *
     * @param options   ==> The list of options that is meant to be filled
     * @param node      ==> current node that will be traversed (DFS)
     */
    void fillOptions(std::vector<std::pair<RRTNode *, RRTOption>> &options, RRTNode *node)
    {
        if (node == nullptr)
        {
            return;
        }

        std::vector<RRTOption> local_options = _dubins.allOptions(node->getPoint(), _goal);

        for (const RRTOption &option : local_options)
        {
            options.emplace_back(std::pair<RRTNode *, RRTOption>{node, option});
        }

        for (RRTNode *child : node->getReachable())
        {
            fillOptions(options, child);
        }
    }

    /**
     * Rewires the tree by finding paths that are more efficintly routed through
     * the sample. Only searches for nodes a specific radius around the sample
     * to reduce computational expense
     *
     * @param sample    ==> sampled point
     */
    void optimizeTree(RRTNode &sample)
    {
        optimizeTreeRecursive(_root, nullptr, sample);
    }

    /**
     *  (RECURSIVE HELPER)
     * Rewires the tree by finding paths that are more efficintly routed through
     * the sample. Only searches for nodes a specific radius around the sample
     * to reduce computational expense
     *
     * @param node      ==> current node (DFS)
     * @param sample    ==> sampled point
     */
    void optimizeTreeRecursive(RRTNode *node, RRTNode *prev_node, RRTNode &sample)
    {
        if (node == nullptr)
        {
            return;
        }

        for (RRTNode *child : node->getReachable())
        {
            optimizeTreeRecursive(child, node, sample);
        }

        // prevents rerouting the root (it shouldn't, but just to be safe)
        if (prev_node == nullptr)
        {
            return;
        }

        // TODO, optimize the tree

        // if the node is not sufficiently close to the sample, dont bother optimizing
        if (node->getPoint().distanceTo(sample.getPoint()) > _rewire_radius)
        {
            return;
        }

        // if this node is the sample, return
        if (node->getPoint() == sample.getPoint())
        {
            return;
        }

        // generate all dubins path from this node
        std::vector<RRTOption> options = _dubins.allOptions(sample.getPoint(), node->getPoint(), true);

        for (const RRTOption &option : options)
        {
            // if the option is not possible, don't bother checking
            if (option.length == std::numeric_limits<double>::infinity())
            {
                break;
            }

            std::vector<XYZCoord> current_path = _dubins.generatePoints(sample.getPoint(), node->getPoint(), option.dubins_path, option.has_straight);
            // the the path is bad, check the next one
            if (!validPath(current_path))
            {
                continue;
            }

            // if the node is uncompetitive, discard it
            if (node->getCost() + option.length >= _tree.getNode(sample.getPoint())->getCost())
            {
                continue;
            }

            // else, replace the existing edge with the new one
            _tree.rewireEdge(&sample, prev_node, node, current_path, sample.getCost() + option.length);
        }
    }

    /**
     * Determines whether a given point is close enough to the goal region
     * (based on some arbitrary tolarance).
     *
     * @param sample    ==> sampled point
     * @return          ==> whether or not the point is in the goal area
     */
    bool inGoalRegion(const RRTPoint &sample)
    {
        return _goal.distanceTo(sample) <= _tolerance_to_goal;
    }

    /**
     * returns a continuous path of points to the goal
     *
     * @return  ==> list of 2-vectors to the goal region
     */
    std::vector<XYZCoord> getPointsGoal()
    {
        return getPointsForPath(getGoalPath());
    }

    /**
     * Given a list of edges in the tree, it returns the list that represents
     * the path formed by connecting each node by dubins curves.
     *
     * @param path  ==> list of edges from the root of the tree to the goal
     *                  node
     * @return      ==> list of points that connects the path through dubins
     *                  curves
     */
    std::vector<XYZCoord> getPointsForPath(std::vector<RRTEdge *> path)
    {
        // TODO, get the Points for the path
        std::vector<XYZCoord> points;

        for (RRTEdge *edge : path)
        {
            for (const XYZCoord &point : edge->getPath())
            {
                points.emplace_back(point);
            }
        }

        return points;
    }

    /**
     * Finds the path from the start position to the goal area through nodes in
     * the tree.
     *
     * @return  ==> list of edges that connect start to goal nodes
     */
    std::vector<RRTEdge *> getGoalPath()
    {
        std::vector<RRTEdge *> path;
        getGoalPathRecursive(_root, path);
        return path;
    }

    /**
     * (RECURSIVE HELPER)
     * Finds the path from the start position to the goal area through nodes in
     * the tree.
     *
     * @param node  ==> the current node (DFS)
     * @param path  ==> the list that is filled by this recursive helper
     */
    void getGoalPathRecursive(RRTNode *node, std::vector<RRTEdge *> &path)
    {
        if (node == nullptr)
        {
            return;
        }

        if (inGoalRegion(node->getPoint()))
        {
            return;
        }

        for (RRTNode *child : node->getReachable())
        {
            path.emplace_back(_tree.getEdge(node->getPoint(), child->getPoint()));
            getGoalPathRecursive(child, path);
        }

        path.pop_back();
    }

private:
    Polygon _bounds;

    RRTNode *_root;
    RRTNode *_goal_node;
    RRTTree _tree;
    RRTPoint _goal;

    Dubins _dubins;

    int _num_iterations;
    double _goal_bias;
    double _search_radius;
    double _distance_to_goal;
    double _tolerance_to_goal;
    double _rewire_radius;

    bool _found_goal;
};

#endif // PATHING_STATIC_HPP_