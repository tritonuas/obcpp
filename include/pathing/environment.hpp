#ifndef INCLUDE_PATHING_ENVIRONMENT_HPP_
#define INCLUDE_PATHING_ENVIRONMENT_HPP_

#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>

#include "utilities/constants.hpp"
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
    Environment(const Polygon valid_region, const std::vector<XYZCoord> goals)
        : valid_region(valid_region),
          goals(goals),
          goals_found(0),
          bounds(findBounds()) {}

    /**
     * Check if a point is in the valid region
     *
     * @param point the point to check
     * @return true if the point is in the valid region, false otherwise
     */
    bool isPointInBounds(XYZCoord point) const { return isPointInPolygon(valid_region, point); }

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
     * Get the goal point
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
     * @return a random RRTPoint in the valid region
     */
    RRTPoint getRandomPoint() const {
        // TODO - use some heuristic to more efficiently generate direction
        // vector (and make it toggleable)

        for (int i = 0; i < TRIES_FOR_RANDOM_POINT; i++) {
            // generates a random point in the rectangle contianing the valid region
            XYZCoord generated_point = {random(bounds.first.first, bounds.first.second),
                                        random(bounds.second.first, bounds.second.second), 0};

            if (isPointInBounds(generated_point)) {
                // print out the random point
                return RRTPoint(generated_point, 0);
            }
        }

        return RRTPoint(goals[goals_found], random(0, TWO_PI));
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

//  private:
    const Polygon valid_region;         // boundary of the valid map
    const std::vector<XYZCoord> goals;  // goal point

    int goals_found;  // whether or not the goal has been found, once it becomes ture, it will never
                      // be false again

    std::pair<std::pair<double, double>, std::pair<double, double>> bounds;  // bounds of the valid
                                                                             // region, first pair
                                                                             // is (min x, max x),
                                                                             // second pair is
                                                                             // (min y, max y)

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
};

#endif  // INCLUDE_PATHING_ENVIRONMENT_HPP_
