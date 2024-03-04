#ifndef INCLUDE_PATHING_ENVIRONMENT_HPP_
#define INCLUDE_PATHING_ENVIRONMENT_HPP_

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

/*
 *  Abstraction of the environment, which is a polygon
 *  that defines the boundary of the map.
 *
 *  [FUTURE]
 *      - add dynamic shrinking and enlarging of the boundary
 *      - add dynamic obstacles
 */
class Environment {
 public:
    Environment(const Polygon& valid_region, const std::vector<XYZCoord>& goals,
                const std::vector<Polygon>& obstacles)
        : valid_region(valid_region),
          goals(goals),
          goals_found(0),
          bounds(findBounds()),
          obstacles(obstacles) {}

    /**
     * Check if a point is in the valid region
     *
     * TODO - analysis if checking all regions for a given point at one time
     * is optimal. The alternative would be to check each region individually
     * for all points
     *
     * @param point the point to check
     * @return true if the point is in the valid region, false otherwise
     */
    bool isPointInBounds(const XYZCoord& point) const {
        if (!isPointInPolygon(valid_region, point)) {
            return false;
        }

        for (const Polygon& obstacle : obstacles) {
            if (isPointInPolygon(obstacle, point)) {
                return false;
            }
        }

        return true;
    }

    /**
     * Check if an entire flight path is in bounds
     *
     * TODO - maybe check based on some interval, to see if the performance increases
     *
     * TODO - do based on dubins curves and not just points
     *
     * @param path the path to check
     * @return true if the path is in bound, false otherwise
     */
    bool isPathInBounds(const std::vector<XYZCoord>& path) const {
        const int center = path.size() / 2;
        const int interval = 5;

        if (!isPointInBounds(path[center])) {
            return false;
        }

        for (int i = 1; i < interval; i++) {
            int count = center + i;

            while (count < path.size()) {
                if (!isPointInBounds(path[count])) {
                    return false;
                }
                count += interval;
            }

            count = center - i;
            while (count >= 0) {
                if (!isPointInBounds(path[count])) {
                    return false;
                }
                count -= interval;
            }
        }

        // for (const XYZCoord& point : path) {
        //     if (!isPointInBounds(point)) {
        //         return false;
        //     }
        // }
        return true;
    }

    bool isPathInBoundsAdv(const std::vector<XYZCoord>& path, const RRTOption& option) const {
        // if (!option.has_straight) {
        //     return isPathInBounds(path);
        // }

        // finds the last point on the first curve, and the first point on the second curve
        // does this using the option, using arclength and the point separation
        const int first_curve_end =
            std::abs(option.dubins_path.beta_0) * TURNING_RADIUS / POINT_SEPARATION + 1;
        const int second_curve_start =
            path.size() - std::abs(option.dubins_path.beta_2) * TURNING_RADIUS / POINT_SEPARATION;

        // sanity check
        if (first_curve_end >= second_curve_start) {
            return isPathInBounds(path);
        }

        if (!isLineInBounds(path[first_curve_end], path[second_curve_start])) {
            return false;
        }

        // checks the points manually in the curve
        for (int i = 0; i <= first_curve_end; i++) {
            if (!isPointInBounds(path[i])) {
                return false;
            }
        }

        for (int i = second_curve_start; i < path.size(); i++) {
            if (!isPointInBounds(path[i])) {
                return false;
            }
        }

        return true;
    }

    /**
     * Get the goal point
     * can be unsafe if goals_found is not in bounds
     *
     * @return the goal point
     */
    XYZCoord getGoal() const { return goals[goals_found]; }

    /**
     * Get the goal point from given index
     *
     * @return the goal point
     */
    XYZCoord getGoal(int index) const { return goals[index]; }

    /**
     * Generate a random point in the valid region
     *
     * The function uniformly selects a point in the valid region, but the probaiblity it actually
     * selects a point depends on how large it is compared to the valid region.
     *
     * @param start_point the startpoint used to generate a random point
     * @param search_radius the radius of the search region
     * @return a random XYZCoord in the valid region
     */
    XYZCoord getRandomPoint() const {
        // TODO - use some heuristic to more efficiently generate direction
        // vector (and make it toggleable)

        for (int i = 0; i < TRIES_FOR_RANDOM_POINT; i++) {
            // generates a random point in the rectangle contianing the valid region
            XYZCoord generated_point = {random(bounds.first.first, bounds.first.second),
                                        random(bounds.second.first, bounds.second.second), 0};

            if (isPointInBounds(generated_point)) {
                // print out the random point
                return generated_point;
            }
        }

        return goals[goals_found];
    }

    /**
     * returns whether or not the goal has been found
     *
     * @return true if the goal has been found, false otherwise
     */
    bool isPathComplete() const { return goals_found == goals.size(); }

    /**
     * returns number of goals found
     *
     * @return number of goals found
     */
    int getGoalsFound() const { return goals_found; }

    /**
     * Sets found_goal to true
     *
     *
     */
    void setGoalfound(int index) { goals_found = std::max(goals_found, index); }

    /**
     * Get number of Goals
     *
     * @return number of goals
     */
    int getNumGoals() const { return goals.size(); }

    /**
     * Determines whether a point ia in this polygon via raycasting. Points
     * on the edge are counted as outside the polygon (to be more
     * conservative)
     *
     * Public ONLY for the sake of testing
     *
     * @param point ==> given point
     * @return      ==> whether or not the point is in this polygon object
     * @see         ==> https://en.wikipedia.org/wiki/Point_in_polygon
     *  [TODO] make a method to augment the polygon to get similar polygons
     *  [TODO] something that increases cost based on time in the edge
     */
    bool isPointInPolygon(const Polygon& polygon, const XYZCoord& point) const {
        bool is_inside = false;

        // point in polygon
        for (int i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
            if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
                (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) /
                                   (polygon[j].y - polygon[i].y) +
                               polygon[i].x)) {
                is_inside = !is_inside;
            }
        }

        return is_inside;
    }

 private:
    const Polygon valid_region;            // boundary of the valid map
    const std::vector<XYZCoord> goals;     // goal point
    const std::vector<Polygon> obstacles;  // obstacles in the map

    int goals_found;  // whether or not the goal has been found, once it becomes ture, it will never
                      // be false again

    const std::pair<std::pair<double, double>, std::pair<double, double>>
        bounds;  // bounds of the valid
                 // region, first pair
                 // is (min x, max x),
                 // second pair is
                 // (min y, max y)

    /**
     * Checks wheter a line segment is in bounds or not, it must NOT intersect
     * either the valid region or the obstacles
     *
     * ASSUMES THAT THE END POINTS ARE IN THE POLYGON
     *
     * @param start_point ==> start point of the line segment
     * @param end_point   ==> end point of the line segment
     * @return            ==> whether or not the line segment is in bounds
     */
    bool isLineInBounds(const XYZCoord& start_point, const XYZCoord& end_point) const {
        if (doesLineIntersectPolygon(start_point, end_point, valid_region)) {
            return false;
        }

        for (const Polygon& obstacle : obstacles) {
            if (doesLineIntersectPolygon(start_point, end_point, obstacle)) {
                return false;
            }
        }

        return true;
    }

    /**
     * Find the bounds of the valid region (i.e. the max/min x and y values).
     *
     * ASSUMES valid_region has already been created
     *
     * @return a pair of pairs, where the first pair is min/max x values and the second is the
     * min/max y values
     */
    std::pair<std::pair<double, double>, std::pair<double, double>> findBounds() {
        // if the region doesn't exist, return a default pair
        // memory safety reasons
        if (valid_region.size() == 0) {
            return std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
        }

        // initialize the bounds as the first point
        const XYZCoord* first_point = &valid_region[0];
        std::pair<double, double> x_bounds = {first_point->x, first_point->x};
        std::pair<double, double> y_bounds = {first_point->y, first_point->y};

        // iterate through the rest of the points and update the bounds
        for (const XYZCoord& point : valid_region) {
            // update x bounds
            if (point.x < x_bounds.first) {
                x_bounds.first = point.x;
            } else if (point.x > x_bounds.second) {
                x_bounds.second = point.x;
            }

            // update y bounds
            if (point.y < y_bounds.first) {
                y_bounds.first = point.y;
            } else if (point.y > y_bounds.second) {
                y_bounds.second = point.y;
            }
        }

        return std::make_pair(x_bounds, y_bounds);
    }

    /**
     * Determines whether a line segment intersects the polygon
     *
     *   @param start_point ==> start point of the line segment
     *   @param end_point   ==> end point of the line segment
     *   @param polygon     ==> polygon to check
     */
    bool doesLineIntersectPolygon(const XYZCoord& start_point, const XYZCoord& end_point,
                                  const Polygon& polygon) const {
        for (int i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
            if (intersect(start_point, end_point, polygon[i], polygon[j])) {
                return true;
            }
        }

        return false;
    }

    /**
     * @see https://stackoverflow.com/questions/3838329/how-can-i-check-if-two-segments-intersect
     * @see https://bryceboe.com/2006/10/23/line-segment-intersection-algorithm/
     */
    bool ccw(const XYZCoord& A, const XYZCoord& B, const XYZCoord& C) const {
        return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
    }

    bool intersect(const XYZCoord& A, const XYZCoord& B, const XYZCoord& C,
                   const XYZCoord& D) const {
        return ccw(A, C, D) != ccw(B, C, D) && ccw(A, B, C) != ccw(A, B, D);
    }
};

#endif  // INCLUDE_PATHING_ENVIRONMENT_HPP_
