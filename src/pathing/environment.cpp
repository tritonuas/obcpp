#include "pathing/environment.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"


/*
To nuke or not to nuke, that is the question
Whether 'tis nobler in the mind to suffer
The pointers and recursion of outrageous fortune,
Or to take arms against a sea of segfaults
And by opposing end them.
*/

// Environment::Environment(const Polygon& valid_region, const Polygon& airdrop_zone,
//                          const Polygon& mapping_region, const std::vector<XYZCoord>& goals,
//                          const std::vector<Polygon>& obstacles)
//     : valid_region(valid_region),
//       airdrop_zone(airdrop_zone),
//       mapping_region(mapping_region),
//       goals(goals),
//       goals_found(0),
//       bounds(findBounds(valid_region)),
//       obstacles(obstacles) {}

// bool Environment::isPointInBounds(const XYZCoord& point) const {
//     if (!isPointInPolygon(valid_region, point)) {
//         return false;
//     }

//     for (const Polygon& obstacle : obstacles) {
//         if (isPointInPolygon(obstacle, point)) {
//             return false;
//         }
//     }

//     return true;
// }

// bool Environment::isPathInBounds(const std::vector<XYZCoord>& path) const {
//     /*
//      *   starts from the center, and walks in botht directions in intervals, this
//      * is motivated by the idea that if an endpoint is in bounds, then the path
//      * is most likely out of bounds in the center of the path. Furthermore,
//      * moving by larger step would be more efficient than checking every point.
//      *
//      * ^only for small step length does this make much sense
//      */
//     const int center = path.size() / 2;
//     const int interval = ENV_PATH_VALIDATION_STEP_SIZE;

//     // special case not checked in the loop
//     if (!isPointInBounds(path[center])) {
//         return false;
//     }

//     for (int i = 1; i < interval; i++) {
//         int count = center + i;

//         // to the right
//         while (count < path.size()) {
//             if (!isPointInBounds(path[count])) {
//                 return false;
//             }
//             count += interval;
//         }

//         // to the left
//         count = center - i;
//         while (count >= 0) {
//             if (!isPointInBounds(path[count])) {
//                 return false;
//             }
//             count -= interval;
//         }
//     }

//     return true;
// }

// // bool Environment::isPathInBoundsAdv(const std::vector<XYZCoord>& path,
// //                                     const RRTOption& option) const {
// //     if (!option.has_straight) {
// //         return isPathInBounds(path);
// //     }

// //     // finds the last point on the first curve, and the first point on the second curve
// //     // does this using the option, using arclength and the point separation
// //     const int first_curve_end =
// //         std::abs(option.dubins_path.beta_0) * TURNING_RADIUS / POINT_SEPARATION + 1;
// //     const int second_curve_start =
// //         path.size() - std::abs(option.dubins_path.beta_2) * TURNING_RADIUS / POINT_SEPARATION;

// //     // sanity check
// //     if (first_curve_end >= second_curve_start) {
// //         return isPathInBounds(path);
// //     }

// //     if (!isLineInBounds(path[first_curve_end], path[second_curve_start])) {
// //         return false;
// //     }

// //     // checks the points manually in the curve
// //     for (int i = 0; i <= first_curve_end; i++) {
// //         if (!isPointInBounds(path[i])) {
// //             return false;
// //         }
// //     }

// //     for (int i = second_curve_start; i < path.size(); i++) {
// //         if (!isPointInBounds(path[i])) {
// //             return false;
// //         }
// //     }

// //     return true;
// // }

// const XYZCoord& Environment::getGoal() const { return goals[goals_found]; }

// const XYZCoord& Environment::getGoal(int index) const { return goals[index]; }

// XYZCoord Environment::getRandomPoint(bool use_mapping_region = false) const {
//     // TODO - use some heuristic to more efficiently generate direction
//     // vector (and make it toggleable)
//     std::pair<std::pair<double, double>, std::pair<double, double>> polygon_bounds = bounds;
//     if (use_mapping_region) {
//         polygon_bounds = findBounds(mapping_region);
//     }

//     for (int i = 0; i < TRIES_FOR_RANDOM_POINT; i++) {
//         // generates a random point in the rectangle contianing the valid region
//         XYZCoord generated_point = {
//             random(polygon_bounds.first.first, polygon_bounds.first.second),
//             random(polygon_bounds.second.first, polygon_bounds.second.second), 0};

//         if (use_mapping_region) {
//             if (isPointInPolygon(mapping_region, generated_point)) {
//                 return generated_point;
//             }
//         } else {
//             if (isPointInBounds(generated_point)) {
//                 return generated_point;
//             }
//         }
//     }

//     return goals[goals_found];
// }

// int Environment::getNumGoals() const { return goals.size(); }

// bool Environment::isPointInPolygon(const Polygon& polygon, const XYZCoord& point) {
//     bool is_inside = false;

//     // point in polygon
//     for (int i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
//         if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
//             (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) /
//                                (polygon[j].y - polygon[i].y) +
//                            polygon[i].x)) {
//             is_inside = !is_inside;
//         }
//     }

//     return is_inside;
// }

// bool Environment::isLineInBounds(const XYZCoord& start_point, const XYZCoord& end_point) const {
//     if (doesLineIntersectPolygon(start_point, end_point, valid_region)) {
//         return false;
//     }

//     for (const Polygon& obstacle : obstacles) {
//         if (doesLineIntersectPolygon(start_point, end_point, obstacle)) {
//             return false;
//         }
//     }

//     return true;
// }

// bool Environment::doesLineIntersectPolygon(const XYZCoord& start_point, const XYZCoord& end_point,
//                                            const Polygon& polygon) const {
//     for (int i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
//         if (intersect(start_point, end_point, polygon[i], polygon[j])) {
//             return true;
//         }
//     }

//     return false;
// }

// // Given three colinear points p, q, r, the function checks if
// // point q lies on line segment 'pr'
// bool Environment::onSegment(XYZCoord p, XYZCoord q, XYZCoord r) const {
//     if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) && q.y <= std::max(p.y, r.y) &&
//         q.y >= std::min(p.y, r.y))
//         return true;
//     return false;
// }

// // To find orientation of ordered triplet (p, q, r).
// // The function returns following values:
// // 0 : Colinear points
// // 1 : Clockwise points
// // 2 : Counterclockwise points
// int Environment::orientation(XYZCoord p, XYZCoord q, XYZCoord r) const {
//     int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
//     if (val == 0) return 0;    // colinear
//     return (val > 0) ? 1 : 2;  // clock or counterclock wise
// }

// // Function to check if segments intersect
// bool Environment::intersect(XYZCoord p1, XYZCoord q1, XYZCoord p2, XYZCoord q2) const {
//     // Find the four orientations needed for general and
//     // special cases
//     int o1 = orientation(p1, q1, p2);
//     int o2 = orientation(p1, q1, q2);
//     int o3 = orientation(p2, q2, p1);
//     int o4 = orientation(p2, q2, q1);

//     // General case
//     if (o1 != o2 && o3 != o4) return true;

//     // Special Cases
//     // p1, q1 and p2 are colinear and p2 lies on segment p1q1
//     if (o1 == 0 && onSegment(p1, p2, q1)) return true;

//     // p1, q1 and q2 are colinear and q2 lies on segment p1q1
//     if (o2 == 0 && onSegment(p1, q2, q1)) return true;

//     // p2, q2 and p1 are colinear and p1 lies on segment p2q2
//     if (o3 == 0 && onSegment(p2, p1, q2)) return true;

//     // p2, q2 and q1 are colinear and q1 lies on segment p2q2
//     if (o4 == 0 && onSegment(p2, q1, q2)) return true;

//     return false;  // Doesn't fall in any of the above cases
// }

// std::vector<XYZCoord> Environment::getAirdropEndpoints(int scan_radius, bool vertical) const {
//     auto bounds = findBounds(airdrop_zone);
//     auto [x_min, x_max] = bounds.first;
//     auto [y_min, y_max] = bounds.second;

//     std::vector<XYZCoord> endpoints;
//     bool fly_down = true;
//     double start = vertical ? x_min + scan_radius : -1 * (y_max - scan_radius);
//     double end = vertical ? x_max - scan_radius : -1 * (y_min + scan_radius);
//     double iteration = scan_radius * 2;

//     for (double coordinate = start; coordinate <= end; coordinate += iteration) {
//         // finds where this x-coordinate intersects with the airdrop zone (always convex)
//         XYZCoord top(0, 0, 0);
//         XYZCoord bottom(0, 0, 0);
//         if (vertical) {
//             top = XYZCoord(coordinate, y_max, 0);
//             bottom = XYZCoord(coordinate, y_min, 0);
//         } else {
//             top = XYZCoord(x_min, -coordinate, 0);
//             bottom = XYZCoord(x_max, -coordinate, 0);
//         }

//         std::vector<XYZCoord> intersections =
//             findIntersections(airdrop_zone, top, bottom, vertical);

//         if (vertical) {
//             if (intersections[0].y < intersections[1].y) {
//                 endpoints.push_back(intersections[1]);
//                 endpoints.push_back(intersections[0]);

//             } else {
//                 endpoints.push_back(intersections[0]);
//                 endpoints.push_back(intersections[1]);
//             }
//         } else {
//             if (intersections[0].x < intersections[1].x) {
//                 endpoints.push_back(intersections[0]);
//                 endpoints.push_back(intersections[1]);

//             } else {
//                 endpoints.push_back(intersections[1]);
//                 endpoints.push_back(intersections[0]);
//             }
//         }
//     }

//     return endpoints;
// }

// std::vector<RRTPoint> Environment::getAirdropWaypoints(int scan_radius, bool one_way,
//                                                        bool vertical) const {
//     std::vector<RRTPoint> waypoints;
//     std::vector<XYZCoord> endpoints = getAirdropEndpoints(scan_radius, vertical);
//     double angle = vertical ? 3.0 / 2.0 * M_PI : 0;

//     if (one_way) {
//         for (const XYZCoord& endpoint : endpoints) {
//             waypoints.push_back(RRTPoint(endpoint, angle));
//         }
//     } else {
//         bool fly_down = true;
//         for (int i = 0; i < endpoints.size(); i += 2) {
//             if (fly_down) {
//                 waypoints.push_back(RRTPoint(endpoints[i], angle));
//                 waypoints.push_back(RRTPoint(endpoints[i + 1], angle));
//             } else {
//                 waypoints.push_back(RRTPoint(endpoints[i + 1], angle));
//                 waypoints.push_back(RRTPoint(endpoints[i], angle));
//             }

//             angle = mod(angle + M_PI, TWO_PI);
//             fly_down = !fly_down;
//         }
//     }

//     return waypoints;
// }

// bool Environment::verticalRayIntersectsEdge(const XYZCoord& p1, const XYZCoord& p2,
//                                             const XYZCoord& rayStart, const XYZCoord& rayEnd,
//                                             XYZCoord& intersection) const {
//     // if the x coordinate lines between the edge
//     if ((p2.x <= rayStart.x && p1.x >= rayStart.x) || (p1.x <= rayStart.x && p2.x >= rayStart.x)) {
//         double slope = (p2.y - p1.y) / (p2.x - p1.x);
//         // finds where they intersect using a line y - y' = m(x - x')
//         intersection = XYZCoord(rayStart.x, slope * (rayStart.x - p1.x) + p1.y, 0);
//         return true;
//     }
//     return false;
// }

// bool Environment::horizontalRayIntersectsEdge(const XYZCoord& p1, const XYZCoord& p2,
//                                               const XYZCoord& rayStart, const XYZCoord& rayEnd,
//                                               XYZCoord& intersection) const {
//     // if the x coordinate lines between the edge
//     if ((p2.y <= rayStart.y && p1.y >= rayStart.y) || (p1.y <= rayStart.y && p2.y >= rayStart.y)) {
//         double inverse_slope = (p2.x - p1.x) / (p2.y - p1.y);
//         // finds where they intersect using a line y - y' = m(x - x')
//         intersection = XYZCoord(inverse_slope * (rayStart.y - p1.y) + p1.x, rayStart.y, 0);
//         return true;
//     }
//     return false;
// }

// std::vector<XYZCoord> Environment::findIntersections(const Polygon& polygon,
//                                                      const XYZCoord& rayStart,
//                                                      const XYZCoord& rayEnd, bool vertical) const {
//     // array to be filled
//     std::vector<XYZCoord> intersections;
//     int n = polygon.size();

//     for (int i = 0; i < n; ++i) {
//         const XYZCoord& p1 = polygon[i];
//         const XYZCoord& p2 = polygon[(i + 1) % n];

//         // temporary variable to be changed if an intersection is found
//         XYZCoord intersection(0, 0, 0);
//         bool found_intersection =
//             vertical ? verticalRayIntersectsEdge(p1, p2, rayStart, rayEnd, intersection)
//                      : horizontalRayIntersectsEdge(p1, p2, rayStart, rayEnd, intersection);
//         if (found_intersection) {
//             intersections.push_back(intersection);
//         }
//     }

//     return intersections;
// }

// std::vector<XYZCoord> Environment::findIntersectionsWithPolygon(const Polygon& polygon,
//                                                                 const XYZCoord& start_point,
//                                                                 const XYZCoord& end_point) const {
//     // for loop through each edge of the polygon
//     // for each edge, find the intersection point with the line segment
//     // if an intersection point is found, add it to the list of intersection
//     // points
//     std::vector<XYZCoord> intersections;
//     for (int i = 0; i < polygon.size(); ++i) {
//         XYZCoord p1 = polygon[i];
//         XYZCoord p2 = polygon[(i + 1) % polygon.size()];
//         if (intersect(start_point, end_point, p1, p2)) {
//             // finds the intersection point
//             double x1 = start_point.x, y1 = start_point.y;
//             double x2 = end_point.x, y2 = end_point.y;
//             double x3 = p1.x, y3 = p1.y;
//             double x4 = p2.x, y4 = p2.y;

//             double a1 = y2 - y1;
//             double b1 = x1 - x2;
//             double c1 = a1 * x1 + b1 * y1;

//             double a2 = y4 - y3;
//             double b2 = x3 - x4;
//             double c2 = a2 * x3 + b2 * y3;

//             double det = a1 * b2 - a2 * b1;

//             double x = (b2 * c1 - b1 * c2) / det;
//             double y = (a1 * c2 - a2 * c1) / det;
//             if (std::find(intersections.begin(), intersections.end(), XYZCoord(x, y, 0)) ==
//                 intersections.end()) {
//                 intersections.push_back(XYZCoord(x, y, 0));
//             }
//         }
//     }

//     return intersections;
// }

// std::pair<double, double> Environment::estimateAreaCoveredAndPathLength(
//     const std::vector<XYZCoord>& new_goals) const {
//     double area_covered = 0.0;
//     double path_length = 0.0;

//     for (int i = 0; i < new_goals.size(); ++i) {
//         XYZCoord start_point = new_goals[i];
//         XYZCoord end_point = new_goals[(i + 1) % new_goals.size()];
//         path_length += start_point.distanceTo(end_point);
//         // Calculates area covered by adding the part of the line segment that is in bounds
//         // Caulcate the intersections and whether the start and end points are in bounds in order to
//         // find the sections of the line that is in bounds
//         if (!doesLineIntersectPolygon(start_point, end_point, mapping_region)) {
//             area_covered += start_point.distanceTo(end_point) * SEARCH_RADIUS * 2;
//         } else {
//             std::vector<XYZCoord> intersections =
//                 findIntersectionsWithPolygon(mapping_region, start_point, end_point);
//             bool start_in_bounds = isPointInPolygon(mapping_region, start_point);
//             bool end_in_bounds = isPointInPolygon(mapping_region, end_point);

//             if (start_in_bounds) {
//                 area_covered += start_point.distanceTo(intersections[0]) * SEARCH_RADIUS;
//                 intersections.erase(intersections.begin());
//             }

//             if (end_in_bounds) {
//                 area_covered += end_point.distanceTo(intersections.back()) * SEARCH_RADIUS;
//                 intersections.pop_back();
//             }

//             for (int i = 0; i < intersections.size(); i += 2) {
//                 area_covered += intersections[i].distanceTo(intersections[i + 1]) * SEARCH_RADIUS;
//             }
//         }
//     }

//     return {area_covered, path_length};
// }

// Polygon Environment::scale(double scale, const Polygon& source_polygon) const {
//     Polygon scaled_polygon;

//     // square bounds of the polygon
//     auto bounds = findBounds(source_polygon);
//     auto [x_min, x_max] = bounds.first;
//     auto [y_min, y_max] = bounds.second;

//     // finds the center of the polygon
//     double x_center = (x_max + x_min) / 2;
//     double y_center = (y_max + y_min) / 2;

//     for (const XYZCoord& point : source_polygon) {
//         // shifts the  polygon to the center so scaling doesn't shift the centter (readability)
//         // scales the polygon
//         // retranslates the center to the original center
//         double scaled_x = (point.x - x_center) * scale + x_center;
//         double scaled_y = (point.y - y_center) * scale + y_center;

//         scaled_polygon.push_back(XYZCoord(scaled_x, scaled_y, 0));
//     }

//     return scaled_polygon;
// }

// std::pair<std::pair<double, double>, std::pair<double, double>> Environment::findBounds(
//     const Polygon& region) const {
//     if (region.empty()) {
//         return std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
//     }

//     // initial values
//     double min_x = region[0].x;
//     double max_x = region[0].x;
//     double min_y = region[0].y;
//     double max_y = region[0].y;

//     // finds the min and max x and y values
//     for (const XYZCoord& point : region) {
//         min_x = std::min(min_x, point.x);
//         max_x = std::max(max_x, point.x);
//         min_y = std::min(min_y, point.y);
//         max_y = std::max(max_y, point.y);
//     }

//     return {{min_x, max_x}, {min_y, max_y}};
// }
