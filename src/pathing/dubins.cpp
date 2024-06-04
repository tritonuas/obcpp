#include "pathing/dubins.hpp"

#include <cassert>
#include <cmath>
#include <limits>

#include "utilities/datatypes.hpp"

/**
 *  Notes from Christopher:
 *
 * TODO - be able to calculate more efficient paths using a turning radius that is larger than the
 * minimum turning radius
 *
 *  The reason everythin is named '0' and '2' instead of 01 or 12
 *  is because
 *      0 - first turn
 *      1 - middle turn (only in [RLR, LRL] pathing)
 *      2 - last turn
 *
 *  Also (for now) the naming isn't great, and explinations for geometry are a bit
 *  shotty, and hard to understand without it being drawn out. Also some of the explinations
 *  for the geometry might be straight up wrong
 *
 *   Additionally, the mod opertor is different in python and C, C will return
 *   negative values in its mod operator
 *      ^ is important, as all distance calculations are done with raw values
 *      of beta this means they need to be positive. if the beta value
 *      represents a right turn, they will be made negative at the very end of
 *      each method. (e.g. if left turn ==> beta
 *      if right turn ==> -beta)
 */

template <typename T>
int sign(T number) {
    return (T(0) < number) - (number < T(0));
}

double mod(double dividend, double divisor) {
    const double cpp_mod = std::fmod(dividend, divisor);

    if (cpp_mod < -0.001) {
        return cpp_mod + divisor;
    }

    return cpp_mod;
    // return std::fmod(std::fmod(dividend, divisor) + divisor, divisor);
}

bool compareRRTOptionLength(const RRTOption &first, const RRTOption &second) {
    return first.length < second.length;
}

XYZCoord findOrthogonalVector2D(const XYZCoord &vector) {
    return XYZCoord{-vector.y, vector.x, vector.z};
}

XYZCoord halfDisplacement(const XYZCoord &vector1, const XYZCoord &vector2) {
    return 0.5 * (vector1 - vector2);
}

Dubins::Dubins(double radius, double point_separation)
    : _radius(radius), _point_separation(point_separation) {
    assert(radius > 0);
    assert(point_separation > 0);
}

XYZCoord Dubins::findCenter(const RRTPoint &point, char side) const {
    assert(side == 'L' || side == 'R');

    // creates a right angle between the RRTPoint vector towards the center
    // left is 90 deg CCW, right is 90 deg CW
    double angle = point.psi + (side == 'L' ? HALF_PI : -HALF_PI);

    // creates the vector offset from the existing position
    return XYZCoord{point.coord.x + (std::cos(angle) * _radius),
                  point.coord.y + (std::sin(angle) * _radius), 0};
}

XYZCoord Dubins::circleArc(const RRTPoint &starting_point, double beta, const XYZCoord &center,
                         double path_length) const {
    // Code is not nessisarily intuitive (I don't want to call sign twice)
    // starting_angle_(+- half_pi depending on the sign) + the_angular_distance_tranveled_(depending
    // on the sign)
    double angle = starting_point.psi + ((path_length / _radius) - HALF_PI) * sign(beta);
    XYZCoord direction_vector{std::cos(angle), std::sin(angle), 0};
    return center + (direction_vector * _radius);
}

std::vector<XYZCoord> Dubins::generatePointsStraight(const RRTPoint &start, const RRTPoint &end,
                                                   const DubinsPath &path) const {
    // the arclength of both curved sections + straight section
    double total_distance =
        _radius * (std::abs(path.beta_0) + std::abs(path.beta_2)) + path.straight_dist;

    XYZCoord center_0 = findCenter(start, (path.beta_0 > 0) ? 'L' : 'R');
    XYZCoord center_2 = findCenter(end, (path.beta_2 > 0) ? 'L' : 'R');

    XYZCoord initial_terminal_point{0, 0, 0};  // start of the straight secton
    XYZCoord final_terminal_point{0, 0, 0};    // end of the straight section

    // finds starting and ending points of the _straight section_
    if (std::abs(path.beta_0) > 0) {
        // Angle the path is going to turn along turn_1
        // (start direction) + (FROM CENTER OF CIRCLE,
        // the angle traveled along curve
        //  * -1 if the plane is turning right)
        double angle = start.psi + (std::abs(path.beta_0) - HALF_PI) * sign(path.beta_0);
        initial_terminal_point = center_0 + _radius * XYZCoord{std::cos(angle), std::sin(angle), 0};
    } else {
        initial_terminal_point = XYZCoord{start.coord.x, start.coord.y, start.coord.z};
    }

    if (std::abs(path.beta_2) > 0) {
        // negative sign before beta_2 is because the path comes in through the back of the end
        // vector
        double angle = end.psi + (-std::abs(path.beta_2) - HALF_PI) * sign(path.beta_2);
        final_terminal_point = center_2 + _radius * XYZCoord{std::cos(angle), std::sin(angle), 0};
    } else {
        final_terminal_point = XYZCoord{end.coord.x, end.coord.y, 0};
    }

    double distance_straight = initial_terminal_point.distanceTo(final_terminal_point);

    //  generates the points for the entire curve.
    std::vector<XYZCoord> points_list;
    for (double current_distance = 0; current_distance < total_distance;
         current_distance += _point_separation) {
        if (current_distance < std::abs(path.beta_0) * _radius) {  // First turn
            points_list.emplace_back(circleArc(start, path.beta_0, center_0, current_distance));
        } else if (current_distance >
                   total_distance - std::abs(path.beta_2) * _radius) {  // Last turn
            // OPTION 1:
            // need to calculate new "start" point, which is the difference in end angle to turn
            // angle RRTPoint final_point_RRT{XYZCoord{final_terminal_point.x,
            // final_terminal_point.y, 0}, end.psi- path.beta_2}; last section is how much distance
            // is covered in second turn points_list.emplace_back(circleArc(final_point_RRT,
            // path.beta_2, center_2, current_distance - (total_distance - std::abs(path.beta_2) *
            // _radius)));

            // OPTION 2:
            // old code, more consise, and clever geometry, but less intuitive
            // the distance is the max negative distance
            // and it gets smaller,
            // so that the angle starts out at
            // final_terminal_point and moves towards the back of end
            points_list.emplace_back(
                circleArc(end, path.beta_2, center_2, current_distance - total_distance));
        } else {  // Straignt Section
            // coefficient is the ratio of the straight distance that has been traversed.
            // (current_distance_traved - (LENGTH_OF_FIRST_TURN_CURVED_PATH)) /
            // length_of_the_straight_path
            double coefficient =
                (current_distance - (std::abs(path.beta_0) * _radius)) / distance_straight;
            // convex linear combination to find the vector along the straight path between the
            // initial and final point https://en.wikiversity.org/wiki/Convex_combination
            points_list.emplace_back(coefficient * final_terminal_point +
                                     (1 - coefficient) * initial_terminal_point);
        }
    }
    points_list.emplace_back(XYZCoord{end.coord.x, end.coord.y, 0});

    return points_list;
}

std::vector<XYZCoord> Dubins::generatePointsCurve(const RRTPoint &start, const RRTPoint &end,
                                                const DubinsPath &path) const {
    // the arclength of all paths
    double total_distance =
        _radius * (std::abs(path.beta_2) + std::abs(path.beta_0) + std::abs(path.straight_dist));

    XYZCoord center_0 = findCenter(start, (path.beta_0 > 0) ? 'L' : 'R');
    XYZCoord center_2 = findCenter(end, (path.beta_2 > 0) ? 'L' : 'R');

    double half_intercenter_distance = center_0.distanceTo(center_2) / 2;
    // uses pythagorean theorem to determine the center
    XYZCoord center_1 =
        0.5 * (center_0 + center_2)  // midpoint
        + sign(path.beta_0)
              // hypotnuse - 2r (distance between center0/2 and center 1)
              // a - intercenter_distance / 2 (half of distance between center 0 and center 2)
              * sqrt(std::pow(2 * _radius, 2) - std::pow(half_intercenter_distance, 2))  //
              * findOrthogonalVector2D(center_2 - center_0)
                    .normalized();  // unit vector (direction vector) orthogonal to the displacement
                                    // vector between the two centers

    // angle between x+ and the "terminal vector" between turn 1 and turn 2
    // i.e the angle from center_1 (x+) to center_0
    // angle of the displacement vector relative to x+ (horizontal) - 180 deg [supplimentary
    // interior angle]
    double psi_0 = std::atan2(center_1.y - center_0.y, center_1.x - center_0.x) - M_PI;

    std::vector<XYZCoord> points_list;
    for (double current_distance = 0; current_distance < total_distance;
         current_distance += _point_separation) {
        if (current_distance < std::abs(path.beta_0) * _radius) {  // First Turn
            points_list.emplace_back(circleArc(start, path.beta_0, center_0, current_distance));
        } else if (current_distance >
                   total_distance - std::abs(path.beta_2) * _radius) {  // Last Turn
            points_list.emplace_back(
                circleArc(end, path.beta_2, center_2, current_distance - total_distance));
        } else {  // Middle Turn
            // angle relative to center 1 pointing around the curve
            // starting angle - (1 if LRL, -1 if RLR, * (total angular distance - angular distance
            // from first turn)) *note, it is subtracting because the sign of the middle curve is
            // always opposite to the start curve
            double angle =
                psi_0 - (sign(path.beta_0) * (current_distance / _radius - std::abs(path.beta_0)));
            XYZCoord direction_vector{std::cos(angle), std::sin(angle), 0};
            points_list.emplace_back(center_1 + _radius * direction_vector);
        }
    }

    points_list.emplace_back(XYZCoord{end.coord.x, end.coord.y, 0});

    return points_list;
}

std::vector<XYZCoord> Dubins::generatePoints(const RRTPoint &start, const RRTPoint &end,
                                           const DubinsPath &path, bool has_straight) const {
    if (has_straight) {
        return generatePointsStraight(start, end, path);
    }

    return generatePointsCurve(start, end, path);
}

RRTOption Dubins::lsl(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                      const XYZCoord &center_2) const {
    double straight_distance = center_0.distanceTo(center_2);

    // angle relative to horizontal
    double alpha = std::atan2(center_2.y - center_0.y, center_2.x - center_0.x);

    // difference in angle on the interval [0, 2pi] (CCW)
    double beta_0 = mod(alpha - start.psi, TWO_PI);
    double beta_2 = mod(end.psi - alpha, TWO_PI);

    double total_distance = _radius * (beta_0 + beta_2) + straight_distance;

    return RRTOption{total_distance, DubinsPath(beta_0, beta_2, straight_distance), true};
}

RRTOption Dubins::rsr(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                      const XYZCoord &center_2) const {
    double straight_distance = center_0.distanceTo(center_2);

    // angle relative to horizontal
    double alpha = std::atan2(center_2.y - center_0.y, center_2.x - center_0.x);

    // offset betwen alpha and other vector [0,2pi]
    // 1] calculates the CCW (positive) rotation from start vector to end vector
    //      (end_a - start_a)
    // 2] takes the negative value of ^
    // 3] ^ % 2pi == the rotation CW in terms of positive radians
    double beta_0 = mod(-(alpha - start.psi), TWO_PI);
    double beta_2 = mod(-(end.psi - alpha), TWO_PI);

    double total_distance = _radius * (beta_2 + beta_0) + straight_distance;

    return RRTOption{total_distance, DubinsPath(-beta_0, -beta_2, straight_distance), true};
}

RRTOption Dubins::lsr(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                      const XYZCoord &center_2) const {
    XYZCoord half_displacement = halfDisplacement(center_2, center_0);
    double psi_0 = std::atan2(half_displacement.y, half_displacement.x);
    double half_intercenter_distance = half_displacement.norm();

    if (half_intercenter_distance < _radius) {
        return RRTOption{std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), true};
    }

    // angle between intercenter displacement vector
    //      AND vector orthogonal to the "leave circle" vector
    double alpha = std::acos(_radius / half_intercenter_distance);
    // (INTERCENTER _reference_) - (ANGLE TO TERMINAL) -
    //      (STARTING VECTOR NORMAL TO CIRCLE)
    // i.e. (angle to terminal relative to horizontal) - (start angle)
    double beta_0 = mod(psi_0 - alpha - (start.psi - HALF_PI), TWO_PI);
    // Same as ^, but shifted PI becuase psi_0 has been shifted PI
    //      (to face the opposite center)
    double beta_2 = mod((M_PI + psi_0) - alpha - (end.psi + HALF_PI), TWO_PI);

    // pythagorean theroem to calculate distance off of known
    //      right trangle using intercenter/radius
    double straight_distance =
        2 * std::sqrt(std::pow(half_intercenter_distance, 2) - std::pow(_radius, 2));
    double total_distance = _radius * (beta_0 + beta_2) + straight_distance;

    return RRTOption{total_distance, DubinsPath(beta_0, -beta_2, straight_distance), true};
}

RRTOption Dubins::rsl(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                      const XYZCoord &center_2) const {
    XYZCoord half_displacement = halfDisplacement(center_2, center_0);
    double psi_0 = std::atan2(half_displacement.y, half_displacement.x);
    double half_intercenter_distance = half_displacement.norm();

    if (half_intercenter_distance < _radius) {
        return RRTOption{std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), true};
    }

    // angle between intercenter displacement vector AND
    //      vector orthogonal to the "leave circle" vector
    double alpha = std::acos(_radius / half_intercenter_distance);
    // (STARTING_ANGLE) - (ANGLE TO TERMINAL)
    //      [assuming everything is relative to horizontal]
    // i.e. (start ==> normal to circle) -
    //      (angle to connect center +
    //          angle between center an "leave circle" point)
    double beta_0 = mod((start.psi + HALF_PI) - (psi_0 + alpha), TWO_PI);
    // Same as ^, but shifted PI becuase psi_0 has been shifted PI
    //  (to face the opposite center)
    double beta_2 = mod((end.psi - HALF_PI) - (alpha + psi_0 + M_PI), TWO_PI);

    // pythagorean theroem to calculate distance off of known
    // right trangle using intercenter/radius
    double straight_distance =
        2 * std::sqrt(std::pow(half_intercenter_distance, 2) - std::pow(_radius, 2));
    double total_distance = _radius * (beta_0 + beta_2) + straight_distance;

    return RRTOption{total_distance, DubinsPath(-beta_0, beta_2, straight_distance), true};
}

RRTOption Dubins::lrl(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                      const XYZCoord &center_2) const {
    double intercenter_distance = center_0.distanceTo(center_2);
    XYZCoord half_displacement = halfDisplacement(center_2, center_0);
    double psi_0 = std::atan2(half_displacement.y, half_displacement.x);

    if (intercenter_distance < 2 * _radius && intercenter_distance > 4 * _radius) {
        return RRTOption{std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), false};
    }

    // angle formed by the connection of the radii (the isosoles triange)
    //      (using similar triangles)
    // 2 * (sin of the triangle made by splitting the isocoles triangle in half
    //      (a similar triangle twice as large))
    //      [half of gamma (splitting the right triangle in two)]
    double gamma = 2 * std::asin(intercenter_distance / (4 * _radius));
    // bottom two angles of isocoles (180 - gamma) = 2 * theta
    double theta = (M_PI - gamma) / 2;
    // (ANGLE REQUIRED TO GET FROM START TO psi_0) + (THETA)
    double beta_0 = mod(psi_0 - (start.psi - HALF_PI) + theta, TWO_PI);
    // (ANGLE FROM psi_0 --> END) + (THETA)
    // [psi_0 now needs to point towards center_0]
    double beta_2 = mod((end.psi - HALF_PI) - (psi_0 + M_PI) + theta, TWO_PI);

    // beta_1 ==> (2PI - gamma) is positive angle that the plane flys through
    double beta_1 = TWO_PI - gamma;
    double total_distance = (beta_1 + beta_0 + beta_2) * _radius;
    return RRTOption{total_distance, DubinsPath(beta_0, beta_2, -beta_1), false};
}

RRTOption Dubins::rlr(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                      const XYZCoord &center_2) const {
    double intercenter_distance = center_0.distanceTo(center_2);
    XYZCoord half_displacement = halfDisplacement(center_2, center_0);
    double psi_0 = std::atan2(half_displacement.y, half_displacement.x);

    if (intercenter_distance < 2 * _radius && intercenter_distance > 4 * _radius) {
        return RRTOption{std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), false};
    }

    // angle formed by the connection of the radii (the isosoles triange)
    //      (using similar triangles)
    // 2 * (sin of the triangle made by splitting the isocoles triangle in half
    //      (a similar triangle twice as large))
    //      [half of gamma (splitting the right triangle in two)]
    double gamma = 2 * std::asin(intercenter_distance / (4 * _radius));
    double theta = (M_PI - gamma) / 2;

    // same as lrl, except its a negative angle because its a right turn
    // (ANGLE REQUIRED TO GET FROM START TO psi_0) + (THETA)
    double beta_0 = mod((start.psi + HALF_PI) - psi_0 + theta, TWO_PI);
    // (ANGLE FROM psi_0 --> END) + (THETA)
    // [psi_0 now needs to point towards center_0]
    double beta_2 = mod((psi_0 + HALF_PI) - end.psi + theta, TWO_PI);

    // beta_1 ==> (2PI - gamma) is positive angle that the plane flys through
    double beta_1 = TWO_PI - gamma;
    double total_distance = (beta_1 + beta_0 + beta_2) * _radius;
    return RRTOption{total_distance, DubinsPath(-beta_0, -beta_2, beta_1), false};
}

std::vector<RRTOption> Dubins::allOptions(const RRTPoint &start, const RRTPoint &end,
                                          bool sort) const {
    XYZCoord center_0_left = findCenter(start, 'L');
    XYZCoord center_0_right = findCenter(start, 'R');
    XYZCoord center_2_left = findCenter(end, 'L');
    XYZCoord center_2_right = findCenter(end, 'R');

    std::vector<RRTOption> options = {
        lsl(start, end, center_0_left, center_2_left),
        rsr(start, end, center_0_right, center_2_right),
        rsl(start, end, center_0_right, center_2_left),
        lsr(start, end, center_0_left, center_2_right)
        //,
        //   lrl(start, end, center_0_left, center_2_left),
        //   rlr(start, end, center_0_right, center_2_right)
    };

    if (sort) {
        std::sort(options.begin(), options.end(), compareRRTOptionLength);
    }

    return options;
}

std::vector<XYZCoord> Dubins::dubinsPath(const RRTPoint &start, const RRTPoint &end) const {
    std::vector<RRTOption> options = allOptions(start, end);
    RRTOption optimal_option =
        *std::min_element(options.begin(), options.end(), compareRRTOptionLength);
    return generatePoints(start, end, optimal_option.dubins_path, optimal_option.has_straight);
}

RRTOption Dubins::bestOption(const RRTPoint &start, const RRTPoint &end) const {
    std::vector<RRTOption> options = allOptions(start, end);
    return *std::min_element(options.begin(), options.end(), compareRRTOptionLength);
}
