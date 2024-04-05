#include "pathing/environment.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

Environment::Environment(const Polygon& valid_region, const Polygon& airdrop_zone,
                         const std::vector<XYZCoord>& goals, const std::vector<Polygon>& obstacles)
    : valid_region(valid_region),
      airdrop_zone(airdrop_zone),
      goals(goals),
      goals_found(0),
      bounds(findBounds(valid_region)),
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

bool Environment::doesLineIntersectPolygon(const XYZCoord& start_point, const XYZCoord& end_point,
                                           const Polygon& polygon) const {
    for (int i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        if (intersect(start_point, end_point, polygon[i], polygon[j])) {
            return true;
        }
    }

    return false;
}

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool Environment::onSegment(XYZCoord p, XYZCoord q, XYZCoord r) const {
    if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) && q.y <= std::max(p.y, r.y) &&
        q.y >= std::min(p.y, r.y))
        return true;
    return false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values:
// 0 : Colinear points
// 1 : Clockwise points
// 2 : Counterclockwise points
int Environment::orientation(XYZCoord p, XYZCoord q, XYZCoord r) const {
    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (val == 0) return 0;    // colinear
    return (val > 0) ? 1 : 2;  // clock or counterclock wise
}

// Function to check if segments intersect
bool Environment::intersect(XYZCoord p1, XYZCoord q1, XYZCoord p2, XYZCoord q2) const {
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if (o1 != o2 && o3 != o4) return true;

    // Special Cases
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;

    // p1, q1 and q2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;

    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;

    // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false;  // Doesn't fall in any of the above cases
}

std::pair<std::pair<double, double>, std::pair<double, double>> Environment::getAirdropBounds()
    const {
    return findBounds(airdrop_zone);
}

std::pair<std::pair<double, double>, std::pair<double, double>> Environment::findBounds(
    const Polygon& region) const {
    if (region.empty()) {
        return std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
    }

    double min_x = region[0].x;
    double max_x = region[0].x;
    double min_y = region[0].y;
    double max_y = region[0].y;

    for (const XYZCoord& point : region) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }

    return {{min_x, max_x}, {min_y, max_y}};
}
