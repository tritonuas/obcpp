#ifndef INCLUDE_PATHING_ENVIRONMENT_HPP_
#define INCLUDE_PATHING_ENVIRONMENT_HPP_

#include <vector>

#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

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
        : valid_region(valid_region),
          goal(goal),
          goal_radius(goal_radius),
          found_goal(false),
          bounds(findBounds()) {}

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

    /**
     * Get the goal point
     *
     * @return the goal point
     */
    RRTPoint getGoal() const { return goal; }

    /**
     * Generate a random point in the valid region
     *
     * @param start_point the startpoint used to generate a random point
     * @param search_radius the radius of the search region
     * @return a random RRTPoint in the valid region
     */
    RRTPoint getRandomPoint() const {
        // TODO - use some heuristic to more efficiently generate direction
        // vector (and make it toggleable)

        // TODO - get rid of magic number
        for (int i = 0; i < 9999; i++) {
            XYZCoord generated_point = {random(bounds.first.first, bounds.first.second),
                                        random(bounds.second.first, bounds.second.second), 0};

            if (isPointInBounds(generated_point)) {
                return RRTPoint(generated_point, random(0, TWO_PI));
            }
        }

        return goal;
    }

    /**
     * returns whether or not the goal has been found
     *
     * @return true if the goal has been found, false otherwise
     */
    bool isGoalFound() const { return found_goal; }

    /**
     * Sets found_goal to true
     */
    void setGoalfound() { found_goal = true; }
 private:
    const Polygon valid_region;  // boundary of the valid map
    const RRTPoint goal;         // goal point
    const double goal_radius;    // radius (tolarance) of the goal region centerd at goal

    bool found_goal;

    std::pair<std::pair<double, double>, std::pair<double, double>> bounds;

    /**
     * Find the bounds of the valid region (i.e. the max/min x and y values).
     *
     * ASSUMES valid_region has already been created
     *
     * @return a pair of pairs, where the first pair is min/max x values and the second is the
     * min/max y values
     */
    std::pair<std::pair<double, double>, std::pair<double, double>> findBounds() {
        if (valid_region.size() == 0) {
            return std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
        }

        const XYZCoord* first_point = &valid_region[0];
        std::pair<double, double> x_bounds = {first_point->x, first_point->x};
        std::pair<double, double> y_bounds = {first_point->y, first_point->y};

        for (const XYZCoord& point : valid_region) {
            x_bounds.first = std::min(x_bounds.first, point.x);
            x_bounds.second = std::max(x_bounds.second, point.x);
            y_bounds.first = std::min(y_bounds.first, point.y);
            y_bounds.second = std::max(y_bounds.second, point.y);
        }

        return std::make_pair(x_bounds, y_bounds);
    }
};

#endif  // INCLUDE_PATHING_ENVIRONMENT_HPP_
