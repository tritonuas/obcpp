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
                const std::vector<Polygon>& obstacles);

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
    bool isPointInBounds(const XYZCoord& point) const;

    /**
     * Check if an entire flight path is in bounds
     *
     * TODO - maybe check based on some interval, to see if the performance increases
     *
     * TODO - do based on dubins curves and not just points
     *
     * @param path the path to check
     * @return true if the path is in bounds, false otherwise
     */
    bool isPathInBounds(const std::vector<XYZCoord>& path) const;

    /**
     *
     * Check if an entire flight path is in bounds
     *
     * Attemps to skip a straight section by checking line segments instead of
     * points, this doesn't actually end up making a large differernce with small
     * path length?
     *
     * @param path the path to check
     * @param option the RRT option associated with the path
     * @return true if the path is in bounds, false otherwise
     */
    bool isPathInBoundsAdv(const std::vector<XYZCoord>& path, const RRTOption& option) const;

    /**
     * Get the goal point
     * can be unsafe if goals_found is not in bounds
     *
     * @return the goal point
     */
    const XYZCoord& getGoal() const;

    /**
     * Get the goal point from given index
     *
     * @return the goal point
     */
    const XYZCoord& getGoal(int index) const;

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
    XYZCoord getRandomPoint() const;

    /**
     * Get number of Goals
     *
     * @return number of goals
     */
    int getNumGoals() const;

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
    bool isPointInPolygon(const Polygon& polygon, const XYZCoord& point) const;

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
    bool isLineInBounds(const XYZCoord& start_point, const XYZCoord& end_point) const;

    /**
     * Find the bounds of the valid region (i.e. the max/min x and y values).
     *
     * ASSUMES valid_region has already been created
     *
     * @return a pair of pairs, where the first pair is min/max x values and the second is the
     * min/max y values
     */
    std::pair<std::pair<double, double>, std::pair<double, double>> findBounds() const;

    /**
     * Determines whether a line segment intersects the polygon
     *
     *   @param start_point ==> start point of the line segment
     *   @param end_point   ==> end point of the line segment
     *   @param polygon     ==> polygon to check
     */
    bool doesLineIntersectPolygon(const XYZCoord& start_point, const XYZCoord& end_point,
                                  const Polygon& polygon) const;

    /**
     * @see https://stackoverflow.com/questions/3838329/how-can-i-check-if-two-segments-intersect
     * @see https://bryceboe.com/2006/10/23/line-segment-intersection-algorithm/
     */
    bool ccw(const XYZCoord& A, const XYZCoord& B, const XYZCoord& C) const;

    /**
     * Determines if two line segments intersect
     * 
     * @param A ==> start point of the first line segment
     * @param B ==> end point of the first line segment
     * @param C ==> start point of the second line segment
     * @param D ==> end point of the second line segment
     * @return  ==> true if intersect, false if not
    */
    bool intersect(const XYZCoord& A, const XYZCoord& B, const XYZCoord& C,
                   const XYZCoord& D) const;
};

#endif  // INCLUDE_PATHING_ENVIRONMENT_HPP_
