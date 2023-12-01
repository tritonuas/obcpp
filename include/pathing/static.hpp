#ifndef PATHING_STATIC_HPP_
#define PATHING_STATIC_HPP_

#include <cmath>
#include <utility>
#include <vector>

#include "core/states.hpp"
#include "pathing/dubins.hpp"
#include "pathing/tree.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

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
    RRT(RRTPoint start, RRTPoint goal, int num_iterations, double goal_bias, double search_radius)
        : _root{start, 0}, _goal{goal}, _num_iterations{num_iterations}, _goal_bias{goal_bias},
          _search_radius{search_radius}, _dubins{TURNING_RADIUS, POINT_SEPARATION}
    {
        _tree.addNode(nullptr, &_root, {}, 0);
    }

    RRTPoint generateSamplePoint(const RRTPoint &from)
    {
        if (random_01() < _goal_bias)
        {
            return RRTPoint{_goal.xyz, random_01() * TWO_PI};
        }

        // if the goal isn't returned, find some angle, and then an arbitrary radius
        double radius = random_01() * _search_radius;
        double angle = random_01() * TWO_PI;

        return RRTPoint{
            XYZCoord{from.xyz.x + std::cos(angle) * radius, from.xyz.y + std::sin(angle) * radius, from.xyz.z}, 0};
    }

    bool validPath(const std::vector<Eigen::Vector2d> &path)
    {
        // polygons dont work yet? also should the be convex?
        for (const Eigen::Vector2d &point : path)
        {
            // todo, add if the point is A] in-bounds B] not inside an obsticle
            if (false)
            {
                return false;
            }
        }

        return true;
    }


    void run()
    {
        _distance_to_goal = std::sqrt(std::pow(_goal.xyz.x - _root.getPoint().xyz.x, 2) + std::pow(_goal.xyz.y - _root.getPoint().xyz.y, 2));
        for (int _; _ < _num_iterations; _++)
        {
            RRTPoint sample = generateSamplePoint(_root.getPoint());

            std::vector<std::pair<RRTNode *, RRTOption>> options = pathingOptions(sample);

            for (const auto &[node, option] : options)
            {
                if (option.length == std::numeric_limits<double>::infinity())
                {
                    break;
                }

                std::vector<Eigen::Vector2d> path = _dubins.generatePoints(_root.getPoint(), sample, option.dubins_path, option.has_straight);

                // TODO: check whats in bounds or not

                // discard point if it is not distinct from the sample
                if (node->getPoint() == sample)
                {
                    break;
                }

                RRTNode new_node{sample, node->getCost() + option.length};
                // If node is uncompetitive, discard it
                if (_tree.getNode(sample) != nullptr && new_node.getCost() >= _tree.getNode(sample)->getCost())
                {
                    continue;
                }

                // else, add the node to the tree.
                _tree.addNode(node, &new_node, _dubins.generatePoints(node->getPoint(), sample, option.dubins_path, option.has_straight), new_node.getCost());

                // TODO, call optimize_tree
            }
        }
    }

    std::vector<std::pair<RRTNode *, RRTOption>> pathingOptions(const RRTPoint &end, int quantity_options = 10)
    {
        std::vector<std::pair<RRTNode *, RRTOption>> options;

        // for (RRTNode *node : // [TODO] need a way to go through every node)
        // {
        //     std::vector<RRTOption> local_options = _dubins.allOptions(node->getPoint(), end);

        //     for (const RRTOption &option : local_options)
        //     {
        //         options.emplace_back(std::pair<RRTNode *, RRTOption>{node, option});
        //     }
        // }

        // std::sort(options.begin(), options.end(), [](std::pair<RRTNode *, RRTOption> a, std::pair<RRTNode *, RRTOption> b)
        //           { return compareRRTOptionLength(a.second, b.second); });

        if (options.size() < quantity_options) {
            return options;
        }

        return {options.begin(), options.begin() + quantity_options};
    }

    // TODO, compare distance to goal with tolarance
    bool inGoalRegion(const RRTPoint &sample) {
        return false;
    }

private:
    RRTNode _root;
    RRTTree _tree;
    RRTPoint _goal;

    Dubins _dubins;

    int _num_iterations;
    double _goal_bias;
    double _search_radius;
    double _distance_to_goal;
};
#endif // PATHING_STATIC_HPP_
