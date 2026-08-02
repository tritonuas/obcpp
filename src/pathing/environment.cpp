#include "pathing/environment.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

namespace Environment {

void init(const Polygon& valid_region, const Polygon& airdrop_zone, const Polygon& mapping_region,
          const std::vector<Polygon>& obstacles) {
    _valid_region = valid_region;
    _airdrop_zone = airdrop_zone;
    _mapping_region = mapping_region;
    _obstacles = obstacles;
    _bounds = findBounds(valid_region);
}

bool isPointInBounds(const XYZCoord& point) {
    if (!isPointInPolygon(_valid_region, point)) {
        return false;
    }

    for (const Polygon& obstacle : _obstacles) {
        if (isPointInPolygon(obstacle, point)) {
            return false;
        }
    }

    return true;
}

bool isPathInBounds(const std::vector<XYZCoord>& path) {
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

bool isDubinsPathInBounds(const RRTPoint& start, const RRTPoint& end, const RRTOption& option) {
    // [LRL, RLR] are disabled in Dubins::allOptions();
    if (!option.has_straight) {
        return false;
    }

    // rejects the sentinel options (infinity) that lsr/rsl produce when no path exists
    if (!std::isfinite(option.length)) {
        return false;
    }

    const double radius = Dubins::_radius;
    const DubinsPath& path = option.dubins_path;

    if (!isPointInBounds(start.coord) || !isPointInBounds(end.coord)) {
        return false;
    }

    // endpoints of the straight section, overwritten by the turn sections below
    XYZCoord straight_start = start.coord;
    XYZCoord straight_end = end.coord;

    // first turn
    if (std::abs(path.beta_0) > 0) {
        const double turn_sign = (path.beta_0 > 0) ? 1.0 : -1.0;
        const XYZCoord center = Dubins::findCenter(start, (turn_sign > 0) ? 'L' : 'R');

        // angle from the center to the plane at the start of the turn
        const double start_angle = start.psi - HALF_PI * turn_sign;
        if (!isArcInBounds(center, radius, start_angle, path.beta_0)) {
            return false;
        }

        const double exit_angle = start.psi + (std::abs(path.beta_0) - HALF_PI) * turn_sign;
        straight_start =
            center + radius * XYZCoord{std::cos(exit_angle), std::sin(exit_angle), 0};
    }

    // last turn (entered backwards -- the arc runs from the end of the straight
    // section to the end vector)
    if (std::abs(path.beta_2) > 0) {
        const double turn_sign = (path.beta_2 > 0) ? 1.0 : -1.0;
        const XYZCoord center = Dubins::findCenter(end, (turn_sign > 0) ? 'L' : 'R');

        // angle from the center to the plane at the start of the turn
        const double entry_angle = end.psi - (std::abs(path.beta_2) + HALF_PI) * turn_sign;
        if (!isArcInBounds(center, radius, entry_angle, path.beta_2)) {
            return false;
        }

        straight_end =
            center + radius * XYZCoord{std::cos(entry_angle), std::sin(entry_angle), 0};
    }

    // straight section
    return isLineInBounds(straight_start, straight_end);
}

bool isArcInBounds(const XYZCoord& center, double radius, double start_angle, double sweep) {
    // an arc that starts in bounds and never crosses a boundary is entirely in bounds
    const XYZCoord arc_start =
        center + radius * XYZCoord{std::cos(start_angle), std::sin(start_angle), 0};
    if (!isPointInBounds(arc_start)) {
        return false;
    }

    if (doesArcIntersectPolygon(center, radius, start_angle, sweep, _valid_region)) {
        return false;
    }

    for (const Polygon& obstacle : _obstacles) {
        if (doesArcIntersectPolygon(center, radius, start_angle, sweep, obstacle)) {
            return false;
        }
    }

    return true;
}

XYZCoord getRandomPoint(bool use_mapping_region, const XYZCoord& fallback) {
    // TODO - use some heuristic to more efficiently generate direction
    // vector (and make it toggleable)
    std::pair<std::pair<double, double>, std::pair<double, double>> polygon_bounds = _bounds;
    if (use_mapping_region) {
        polygon_bounds = findBounds(_mapping_region);
    }

    for (int i = 0; i < TRIES_FOR_RANDOM_POINT; i++) {
        // generates a random point in the rectangle contianing the valid region
        XYZCoord generated_point = {
            random(polygon_bounds.first.first, polygon_bounds.first.second),
            random(polygon_bounds.second.first, polygon_bounds.second.second), 0};

        if (use_mapping_region) {
            if (isPointInPolygon(_mapping_region, generated_point)) {
                return generated_point;
            }
        } else {
            if (isPointInBounds(generated_point)) {
                return generated_point;
            }
        }
    }

    return fallback;
}

bool isPointInPolygon(const Polygon& polygon, const XYZCoord& point) {
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

bool isPolygonInPolygon(const Polygon& inner, const Polygon& outer) {
    for (const XYZCoord& corner : inner) {
        if (!isPointInPolygon(outer, corner)) {
            return false;
        }
    }

    // an edge can bulge out between two corners that are both inside, which
    // shows up as it crossing the outer boundary
    for (std::size_t i = 0, j = inner.size() - 1; i < inner.size(); j = i++) {
        if (doesLineIntersectPolygon(inner[j], inner[i], outer)) {
            return false;
        }
    }

    return true;
}

bool isLineInBounds(const XYZCoord& start_point, const XYZCoord& end_point) {
    if (doesLineIntersectPolygon(start_point, end_point, _valid_region)) {
        return false;
    }

    for (const Polygon& obstacle : _obstacles) {
        if (doesLineIntersectPolygon(start_point, end_point, obstacle)) {
            return false;
        }
    }

    return true;
}

bool doesLineIntersectPolygon(const XYZCoord& start_point, const XYZCoord& end_point,
                              const Polygon& polygon) {
    for (int i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        if (intersect(start_point, end_point, polygon[i], polygon[j])) {
            return true;
        }
    }

    return false;
}

bool doesArcIntersectPolygon(const XYZCoord& center, double radius, double start_angle,
                             double sweep, const Polygon& polygon) {
    for (int i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        if (doesArcIntersectSegment(center, radius, start_angle, sweep, polygon[i], polygon[j])) {
            return true;
        }
    }

    return false;
}

bool doesArcIntersectSegment(const XYZCoord& center, double radius, double start_angle,
                             double sweep, const XYZCoord& seg_start, const XYZCoord& seg_end) {
    // parameterize the segment as P(t) = seg_start + t * d, t in [0, 1], and solve
    // |P(t) - center|^2 = radius^2, a quadratic in t
    // @see https://stackoverflow.com/questions/1073336/circle-line-segment-collision-detection
    const XYZCoord d = seg_end - seg_start;
    const XYZCoord f = seg_start - center;

    // 2D only -- z is ignored, matching the rest of the environment checks
    const double a = d.x * d.x + d.y * d.y;
    const double b = 2 * (f.x * d.x + f.y * d.y);
    const double c = f.x * f.x + f.y * f.y - radius * radius;

    const double discriminant = b * b - 4 * a * c;
    if (a == 0 || discriminant < 0) {  // degenerate segment or no circle intersection
        return false;
    }

    const double sqrt_discriminant = std::sqrt(discriminant);
    for (const double t : {(-b - sqrt_discriminant) / (2 * a),
                           (-b + sqrt_discriminant) / (2 * a)}) {
        // hit must be within the segment
        if (t < 0 || t > 1) {
            continue;
        }

        // hit must be within the arc's angular range: walk from start_angle in the sweep
        // direction and see if the hit is reached before the sweep is used up
        const double theta =
            std::atan2(f.y + t * d.y, f.x + t * d.x);  // angle of hit relative to center
        const double travelled = (sweep > 0) ? mod(theta - start_angle, TWO_PI)
                                             : mod(start_angle - theta, TWO_PI);
        if (travelled <= std::abs(sweep)) {
            return true;
        }
    }

    return false;
}

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool onSegment(XYZCoord p, XYZCoord q, XYZCoord r) {
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
int orientation(XYZCoord p, XYZCoord q, XYZCoord r) {
    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (val == 0) return 0;    // colinear
    return (val > 0) ? 1 : 2;  // clock or counterclock wise
}

// Function to check if segments intersect
bool intersect(XYZCoord p1, XYZCoord q1, XYZCoord p2, XYZCoord q2) {
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

std::vector<XYZCoord> getAirdropEndpoints(int scan_radius, bool vertical) {
    auto zone_bounds = findBounds(_airdrop_zone);
    auto [x_min, x_max] = zone_bounds.first;
    auto [y_min, y_max] = zone_bounds.second;

    std::vector<XYZCoord> endpoints;
    double start = vertical ? x_min + scan_radius : -1 * (y_max - scan_radius);
    double end = vertical ? x_max - scan_radius : -1 * (y_min + scan_radius);
    double iteration = scan_radius * 2;

    for (double coordinate = start; coordinate <= end; coordinate += iteration) {
        // finds where this x-coordinate intersects with the airdrop zone (always convex)
        XYZCoord top(0, 0, 0);
        XYZCoord bottom(0, 0, 0);
        if (vertical) {
            top = XYZCoord(coordinate, y_max, 0);
            bottom = XYZCoord(coordinate, y_min, 0);
        } else {
            top = XYZCoord(x_min, -coordinate, 0);
            bottom = XYZCoord(x_max, -coordinate, 0);
        }

        std::vector<XYZCoord> intersections =
            findIntersections(_airdrop_zone, top, bottom, vertical);

        if (vertical) {
            if (intersections[0].y < intersections[1].y) {
                endpoints.push_back(intersections[1]);
                endpoints.push_back(intersections[0]);

            } else {
                endpoints.push_back(intersections[0]);
                endpoints.push_back(intersections[1]);
            }
        } else {
            if (intersections[0].x < intersections[1].x) {
                endpoints.push_back(intersections[0]);
                endpoints.push_back(intersections[1]);

            } else {
                endpoints.push_back(intersections[1]);
                endpoints.push_back(intersections[0]);
            }
        }
    }

    return endpoints;
}

std::vector<RRTPoint> getAirdropWaypoints(int scan_radius, bool one_way, bool vertical) {
    std::vector<RRTPoint> waypoints;
    std::vector<XYZCoord> endpoints = getAirdropEndpoints(scan_radius, vertical);
    double angle = vertical ? 3.0 / 2.0 * M_PI : 0;

    if (one_way) {
        for (const XYZCoord& endpoint : endpoints) {
            waypoints.push_back(RRTPoint(endpoint, angle));
        }
    } else {
        bool fly_down = true;
        for (int i = 0; i < endpoints.size(); i += 2) {
            if (fly_down) {
                waypoints.push_back(RRTPoint(endpoints[i], angle));
                waypoints.push_back(RRTPoint(endpoints[i + 1], angle));
            } else {
                waypoints.push_back(RRTPoint(endpoints[i + 1], angle));
                waypoints.push_back(RRTPoint(endpoints[i], angle));
            }

            angle = mod(angle + M_PI, TWO_PI);
            fly_down = !fly_down;
        }
    }

    return waypoints;
}

bool verticalRayIntersectsEdge(const XYZCoord& p1, const XYZCoord& p2, const XYZCoord& rayStart,
                               const XYZCoord& rayEnd, XYZCoord& intersection) {
    // if the x coordinate lines between the edge
    if ((p2.x <= rayStart.x && p1.x >= rayStart.x) || (p1.x <= rayStart.x && p2.x >= rayStart.x)) {
        double slope = (p2.y - p1.y) / (p2.x - p1.x);
        // finds where they intersect using a line y - y' = m(x - x')
        intersection = XYZCoord(rayStart.x, slope * (rayStart.x - p1.x) + p1.y, 0);
        return true;
    }
    return false;
}

bool horizontalRayIntersectsEdge(const XYZCoord& p1, const XYZCoord& p2, const XYZCoord& rayStart,
                                 const XYZCoord& rayEnd, XYZCoord& intersection) {
    // if the x coordinate lines between the edge
    if ((p2.y <= rayStart.y && p1.y >= rayStart.y) || (p1.y <= rayStart.y && p2.y >= rayStart.y)) {
        double inverse_slope = (p2.x - p1.x) / (p2.y - p1.y);
        // finds where they intersect using a line y - y' = m(x - x')
        intersection = XYZCoord(inverse_slope * (rayStart.y - p1.y) + p1.x, rayStart.y, 0);
        return true;
    }
    return false;
}

std::vector<XYZCoord> findIntersections(const Polygon& polygon, const XYZCoord& rayStart,
                                        const XYZCoord& rayEnd, bool vertical) {
    // array to be filled
    std::vector<XYZCoord> intersections;
    int n = polygon.size();

    for (int i = 0; i < n; ++i) {
        const XYZCoord& p1 = polygon[i];
        const XYZCoord& p2 = polygon[(i + 1) % n];

        // temporary variable to be changed if an intersection is found
        XYZCoord intersection(0, 0, 0);
        bool found_intersection =
            vertical ? verticalRayIntersectsEdge(p1, p2, rayStart, rayEnd, intersection)
                     : horizontalRayIntersectsEdge(p1, p2, rayStart, rayEnd, intersection);
        if (found_intersection) {
            intersections.push_back(intersection);
        }
    }

    return intersections;
}

std::vector<XYZCoord> findIntersectionsWithPolygon(const Polygon& polygon,
                                                   const XYZCoord& start_point,
                                                   const XYZCoord& end_point) {
    // for loop through each edge of the polygon
    // for each edge, find the intersection point with the line segment
    // if an intersection point is found, add it to the list of intersection
    // points
    std::vector<XYZCoord> intersections;
    for (int i = 0; i < polygon.size(); ++i) {
        XYZCoord p1 = polygon[i];
        XYZCoord p2 = polygon[(i + 1) % polygon.size()];
        if (intersect(start_point, end_point, p1, p2)) {
            // finds the intersection point
            double x1 = start_point.x, y1 = start_point.y;
            double x2 = end_point.x, y2 = end_point.y;
            double x3 = p1.x, y3 = p1.y;
            double x4 = p2.x, y4 = p2.y;

            double a1 = y2 - y1;
            double b1 = x1 - x2;
            double c1 = a1 * x1 + b1 * y1;

            double a2 = y4 - y3;
            double b2 = x3 - x4;
            double c2 = a2 * x3 + b2 * y3;

            double det = a1 * b2 - a2 * b1;

            double x = (b2 * c1 - b1 * c2) / det;
            double y = (a1 * c2 - a2 * c1) / det;
            if (std::find(intersections.begin(), intersections.end(), XYZCoord(x, y, 0)) ==
                intersections.end()) {
                intersections.push_back(XYZCoord(x, y, 0));
            }
        }
    }

    return intersections;
}

Polygon scale(double scale, const Polygon& source_polygon) {
    Polygon scaled_polygon;

    // square bounds of the polygon
    auto polygon_bounds = findBounds(source_polygon);
    auto [x_min, x_max] = polygon_bounds.first;
    auto [y_min, y_max] = polygon_bounds.second;

    // finds the center of the polygon
    double x_center = (x_max + x_min) / 2;
    double y_center = (y_max + y_min) / 2;

    for (const XYZCoord& point : source_polygon) {
        // shifts the  polygon to the center so scaling doesn't shift the centter (readability)
        // scales the polygon
        // retranslates the center to the original center
        double scaled_x = (point.x - x_center) * scale + x_center;
        double scaled_y = (point.y - y_center) * scale + y_center;

        scaled_polygon.push_back(XYZCoord(scaled_x, scaled_y, 0));
    }

    return scaled_polygon;
}

std::pair<std::pair<double, double>, std::pair<double, double>> findBounds(const Polygon& region) {
    if (region.empty()) {
        return std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
    }

    // initial values
    double min_x = region[0].x;
    double max_x = region[0].x;
    double min_y = region[0].y;
    double max_y = region[0].y;

    // finds the min and max x and y values
    for (const XYZCoord& point : region) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }

    return {{min_x, max_x}, {min_y, max_y}};
}

}  // namespace Environment
