#include "pathing/environment.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

Environment::Environment(const Polygon& valid_region, const std::vector<XYZCoord>& goals,
                         const std::vector<Polygon>& obstacles)
    : valid_region(valid_region),
      goals(goals),
      goals_found(0),
      bounds(findBounds()),
      obstacles(obstacles) {}

bool Environment::isPointInBounds(const XYZCoord& point) const {
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

bool Environment::isPathInBounds(const std::vector<XYZCoord>& path) const {
    /*
     *   starts from the center, and walks in botht directions in intervals, this
     * is motivated by the idea that if an endpoint is in bounds, then the path
     * is most likely out of bounds in the center of the path. Furthermore,
     * moving by larger step would be more efficient than checking every point.
     *
     * ^only for small step length does this make much sense
     */
    const int center = path.size() / 2;
    const int interval = ENV_PATH_VALIDATION_STEP_SIZE;

    // special case not checked in the loop
    if (!isPointInBounds(path[center])) {
        return false;
    }

    for (int i = 1; i < interval; i++) {
        int count = center + i;

        // to the right
        while (count < path.size()) {
            if (!isPointInBounds(path[count])) {
                return false;
            }
            count += interval;
        }

        // to the left
        count = center - i;
        while (count >= 0) {
            if (!isPointInBounds(path[count])) {
                return false;
            }
            count -= interval;
        }
    }

    return true;
}

bool Environment::isPathInBoundsAdv(const std::vector<XYZCoord>& path,
                                    const RRTOption& option) const {
    if (!option.has_straight) {
        return isPathInBounds(path);
    }

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

const XYZCoord& Environment::getGoal() const { return goals[goals_found]; }

const XYZCoord& Environment::getGoal(int index) const { return goals[index]; }

XYZCoord Environment::getRandomPoint() const {
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

int Environment::getNumGoals() const { return goals.size(); }

bool Environment::isPointInPolygon(const Polygon& polygon, const XYZCoord& point) const {
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

bool Environment::isLineInBounds(const XYZCoord& start_point, const XYZCoord& end_point) const {
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

std::pair<std::pair<double, double>, std::pair<double, double>> Environment::findBounds() const {
    double min_x = valid_region[0].x;
    double max_x = valid_region[0].x;
    double min_y = valid_region[0].y;
    double max_y = valid_region[0].y;

    for (const XYZCoord& point : valid_region) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }

    return std::make_pair(std::make_pair(min_x, max_x), std::make_pair(min_y, max_y));
}

bool Environment::doesLineIntersectPolygon(const XYZCoord& start_point, const XYZCoord& end_point,
                                           const Polygon& polygon) const {
    for (int i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        if (intersect(start_point, end_point, polygon[i], polygon[j])) {
            return true;
        }
    }

    return false;
}

bool Environment::ccw(const XYZCoord& A, const XYZCoord& B, const XYZCoord& C) const {
    return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
}

bool Environment::intersect(const XYZCoord& A, const XYZCoord& B, const XYZCoord& C,
                            const XYZCoord& D) const {
    return ccw(A, C, D) != ccw(B, C, D) && ccw(A, B, C) != ccw(A, B, D);
}