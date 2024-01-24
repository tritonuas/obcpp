#ifndef INCLUDE_PATHING_ENVIRONMENT_HPP_
#define INCLUDE_PATHING_ENVIRONMENT_HPP_

#include "utilities/datatypes.hpp"

/*
 *  Abstracction of the environment, which is a polygon
 *  that defines the boundary of the map.
 *
 *
 *  [FUTURE]
 *      - add dynamic shrinking and enlarging of the boundary
 *      - add dynamic obstacles
 */
class Environment {
 public:
    Environment(const Polygon valid_region, const RRTPoint goal, const double goal_radius)
        : valid_region(valid_region), goal(goal), goal_radius(goal_radius) {}

    /**
     * Check if a point is in the valid region
     *
     * @param point the point to check
     * @return true if the point is in the valid region, false otherwise
     */
    bool isPointInBounds(XYZCoord point) const { return valid_region.isPointInBounds(point); }

    /**
     * Check if an entire flight path is in bounds
     *
     * @param path the path to check
     * @return true if the path is in bound, false otherwise
     */
    bool isPathInBounds(const std::vector<XYZCoord>& path) const {
        for (const XYZCoord& point : path) {
            if (!isPointInBounds(point)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Check if a point is in the goal region
     *
     * @param point the point to check
     * @return true if the point is in the goal region, false otherwise
     */
    bool isPointInGoal(XYZCoord point) const {
        double x = point.x - goal.coord.x;
        double y = point.y - goal.coord.y;
        double norm = sqrt(x * x + y * y);
        return norm <= goal_radius;
    }

 private:
    const Polygon valid_region;  // boundary of the valid map
    const RRTPoint goal;         // goal point
    const double goal_radius;    // radius (tolarance) of the goal region centerd at goal

    bool found_goal;
};

#endif  // INCLUDE_PATHING_ENVIRONMENT_HPP_
