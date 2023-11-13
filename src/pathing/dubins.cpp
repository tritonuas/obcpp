
#include <cmath>
#include <limits>

#include "Eigen"

#include "../../include/utilities/datatypes.hpp"
#include "../../include/pathing/dubins.hpp"

/**
 *   Notes from Christopher:
 *
 *   The reason everythin is named '0' and '2' instead of 01 or 12
 *   is because
 *       0 - first turn
 *       1 - middle turn (only in [RLR, LRL] pathing)
 *       2 - last turn
 *
 *   Also (for now) the naming is very inconsistant throughout the file.
 *   This inconsistant naming was also present in the original dubins.py on
 *   the original OBC (the code was also inconsistant in styling)
 *
 *   Additionally, the mod opertor is different in python and C, C will return
 *   negative values in its mod operator
 *      ^ is important, as all distance calculations are done with raw values of beta
 *      this means they need to be positive. if the beta value represents a right turn,
 *      they will be made negative at the very end of each method. (e.g. if left turn ==> beta
 *      if right turn ==> -beta)
 */

template <typename T>
int sign(T val)
{
    return (T(0) < val) - (val < T(0));
}

double mod(double a, double n)
{
    return std::fmod(std::fmod(a, n) + n, n);
}

bool compareRRTOptionLength(const RRTOption &first, const RRTOption &second)
{
    return first.length < second.length;
}

Eigen::Vector2d findOrthogonalVector2D(const Eigen::Vector2d &vector)
{
    return Eigen::Vector2d(-vector.y(), vector.x());
}

double distanceBetween(const Eigen::Vector2d &vector1, const Eigen::Vector2d &vector2)
{
    return (vector1 - vector2).norm();
}

Eigen::Vector2d halfDisplacement(const Eigen::Vector2d &vector1, const Eigen::Vector2d &vector2)
{
    return (vector1 - vector2) / 2;
}

Dubins::Dubins(double radius, double point_separation)
    : _radius(radius), _point_separation(point_separation)
{
    assert(radius > 0);
    assert(point_separation > 0);
}

Eigen::Vector2d Dubins::findCenter(const XYZCoord &point, char side)
{
    assert(side == 'L' || side == 'R');

    // creates a right angle between the XYZCoord vector towards the center
    // left is 90 deg CCW, right is 90 deg CW
    double angle = point.psi + (side == 'L' ? HALF_PI : -HALF_PI);

    // creates the vector offset from the existing position
    return Eigen::Vector2d(point.x + (std::cos(angle) * _radius),
                           point.y + (std::sin(angle) * _radius));
}

Eigen::Vector2d Dubins::circleArc(const XYZCoord &starting_point, double beta, const Eigen::Vector2d &center, double path_length)
{
    // forward angle + [(displacement angle pointing from the center of the turning curcle)
    // * -1 IF angle is negative ELSE 1]
    double angle = starting_point.psi + ((path_length / _radius) - HALF_PI) * sign(beta);
    Eigen::Vector2d angle_vector(std::cos(angle), std::sin(angle));
    return center + (angle_vector * _radius);
}

std::vector<Eigen::Vector2d> Dubins::generatePointsStraight(const XYZCoord &start, const XYZCoord &end, const DubinsPath &path)
{
    // the arclength of both curved sections + straight section
    double total_distance = _radius * (std::abs(path.beta_0) + std::abs(path.beta_2)) + path.straight_dist;

    Eigen::Vector2d center_0 = findCenter(start, (path.beta_0 > 0) ? 'L' : 'R');
    Eigen::Vector2d center_2 = findCenter(end, (path.beta_2 > 0) ? 'L' : 'R');

    Eigen::Vector2d initial_point(0, 0); // start of the straight secton
    Eigen::Vector2d final_point(0, 0);   // end of the straight section

    // finds starting and ending points of the _straight section_
    if (std::abs(path.beta_0) > 0)
    {
        // Angle the path is going to turn along turn_1
        // (start direction) + (FROM CENTER OF CIRCLE, the angle traveled along curve
        //  * -1 if the plane is turning right)
        double angle = start.psi + (std::abs(path.beta_0) - HALF_PI) * sign(path.beta_0);
        initial_point = center_0 + _radius * Eigen::Vector2d(std::cos(angle), std::sin(angle));
    }
    else
    {
        initial_point = Eigen::Vector2d(start.x, start.y);
    }

    if (std::abs(path.beta_2) > 0)
    {
        // negative sign before beta_2 is because the path comes in through the back of the end vector
        double angle = end.psi + (-std::abs(path.beta_2) - HALF_PI) * sign(path.beta_2);
        final_point = center_2 + _radius * Eigen::Vector2d(std::cos(angle), std::sin(angle));
    }
    else
    {
        final_point = Eigen::Vector2d(end.x, end.y);
    }

    double distance_straight = distanceBetween(initial_point, final_point);

    //  generates the points for the entire curve.
    std::vector<Eigen::Vector2d> points_list;
    for (int current_distance = 0; current_distance < total_distance; current_distance += _point_separation)
    {
        if (current_distance < std::abs(path.beta_0) * _radius)
        { // First turn
            points_list.push_back(circleArc(start, path.beta_0, center_0, current_distance));
        }
        else if (current_distance > total_distance - std::abs(path.beta_2) * _radius)
        { // Last turn
            // need to calculate new "start" point, which is the difference in end angle to turn angle
            XYZCoord final_point_XYZ(final_point.x(), final_point.y(), 0, end.psi - path.beta_2);
            // last section is how much distance is covered in second turn
            points_list.push_back(circleArc(final_point_XYZ, path.beta_2, center_2, current_distance - (total_distance - std::abs(path.beta_2) * _radius)));

            // old code, more consise, and clever geometry, but less intuitive
            // the distance is the max negative distance and it gets smaller, so that the angle starts out at
            // final_point and moves towards the back of end
            // points_list.push_back(circleArc(end, path.beta_2, center_2, current_distance - total_distance));
        }
        else
        { // Straignt Section
            // coefficient is the ratio of the straight distance that has been traversed.
            // (current_distance_traved - (LENGTH_OF_FIRST_TURN_CURVED_PATH)) / length_of_the_straight_path
            double coefficient = (current_distance - (std::abs(path.beta_0) * _radius)) / distance_straight;
            // convex linear combination to find the vector along the straight path between the initial and final point
            // https://en.wikiversity.org/wiki/Convex_combination
            points_list.push_back(coefficient * final_point + (1 - coefficient) * initial_point);
        }
    }
    points_list.push_back(Eigen::Vector2d(end.x, end.y));

    return points_list;
}

std::vector<Eigen::Vector2d> Dubins::generatePointsCurve(const XYZCoord &start, const XYZCoord &end, const DubinsPath &path)
{
    // the arclength of all paths
    double total_distance = _radius * (std::abs(path.beta_2) + std::abs(path.beta_0) + std::abs(path.straight_dist));

    Eigen::Vector2d center_0 = findCenter(start, (path.beta_0 > 0) ? 'L' : 'R');
    Eigen::Vector2d center_2 = findCenter(end, (path.beta_2 > 0) ? 'L' : 'R');

    double half_intercenter_distance = distanceBetween(center_0, center_2) / 2;
    // uses pythagorean theorem to determine the center
    Eigen::Vector2d center_1 = ((center_0 + center_2) / 2) // midpoint
                               + sign(path.beta_0)
                                     // hypotnuse - 2r (distance between center0/2 and center 1)
                                     // a - intercenter_distance / 2 (half of distance between center 0 and center 2)
                                     * sqrt(std::pow(2 * _radius, 2) - std::pow(half_intercenter_distance, 2)) //
                                     * findOrthogonalVector2D(center_2 - center_0).normalized();               // unit vector (direction vector) orthogonal to the displacement vector between the two centers

    // angle between x+ and the "handoff vector" between turn 1 and turn 2
    // i.e the angle from center_1 (x+) to center_0
    // angle of the displacement vector relative to x+ (horizontal) - 180 deg [supplimentary interior angle]
    double psi_0 = std::atan2(center_1.y() - center_0.y(), center_1.x() - center_0.x()) - M_PI;

    std::vector<Eigen::Vector2d> points_list;
    for (double current_distance = 0; current_distance < total_distance; current_distance += _point_separation)
    {
        if (current_distance < std::abs(path.beta_0) * _radius)
        { // First Turn
            points_list.push_back(circleArc(start, path.beta_0, center_0, current_distance));
        }
        else if (current_distance > total_distance - std::abs(path.beta_2) * _radius)
        { // Last Turn
            points_list.push_back(circleArc(end, path.beta_2, center_2, current_distance - total_distance));
        }
        else
        { // Middle Turn
            // angle relative to center 1 pointing around the curve
            // starting angle - (1 if LRL, -1 if RLR, * (total angular distance - angular distance from first turn))
            // *note, it is subtracting because the sign of the middle curve is always opposite to the start curve
            double angle = psi_0 - (sign(path.beta_0) * (current_distance / _radius - std::abs(path.beta_0)));
            Eigen::Vector2d vect(std::cos(angle), std::sin(angle));
            points_list.push_back(center_1 + _radius * vect);
        }
    }

    points_list.push_back(Eigen::Vector2d(end.x, end.y));

    return points_list;
}

std::vector<Eigen::Vector2d> Dubins::generatePoints(const XYZCoord &start, const XYZCoord &end, const DubinsPath &path, bool has_straight)
{
    if (has_straight)
    {
        return generatePointsStraight(start, end, path);
    }
    else
    {
        return generatePointsCurve(start, end, path);
    }
}

RRTOption Dubins::lsl(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
{
    double straight_distance = distanceBetween(center_0, center_2);

    // angle relative to horizontal
    double alpha = std::atan2(center_2.y() - center_0.y(), center_2.x() - center_0.x());

    // difference in angle on the interval [0, 2pi] (CCW)
    double beta_0 = mod(alpha - start.psi, TWO_PI);
    double beta_2 = mod(end.psi - alpha, TWO_PI);

    double total_distance = _radius * (beta_0 + beta_2) + straight_distance;

    return RRTOption(total_distance, DubinsPath(beta_0, beta_2, straight_distance), true);
}

RRTOption Dubins::rsr(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
{
    double straight_distance = distanceBetween(center_0, center_2);

    // angle relative to horizontal
    double alpha = std::atan2(center_2.y() - center_0.y(), center_2.x() - center_0.x());

    // offset betwen alpha and other vector [0,2pi]
    // 1] calculates the CCW (positive) rotation from start vector to end vector (end_a - start_a)
    // 2] takes the negative value of ^
    // 3] ^ % 2pi == the rotation CW in terms of positive radians
    double beta_0 = mod(-(alpha - start.psi), TWO_PI);
    double beta_2 = mod(-(end.psi - alpha), TWO_PI);

    double total_distance = _radius * (beta_2 + beta_0) + straight_distance;

    return RRTOption(total_distance, DubinsPath(-beta_0, -beta_2, straight_distance), true);
}

RRTOption Dubins::lsr(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
{
    Eigen::Vector2d half_displacement = halfDisplacement(center_2, center_0);
    double psi_0 = std::atan2(half_displacement.y(), half_displacement.x());
    double half_intercenter_distance = half_displacement.norm();

    if (half_intercenter_distance < _radius)
    {
        return RRTOption(std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), true);
    }

    // angle between intercenter displacement vector AND vector orthogonal to the "leave circle" vector
    double alpha = std::acos(_radius / half_intercenter_distance);
    // (INTERCENTER _reference_) - (ANGLE TO TERMINAL) - (STARTING VECTOR NORMAL TO CIRCLE)
    // i.e. (angle to terminal relative to horizontal) - (start angle)
    double beta_0 = mod(psi_0 - alpha - (start.psi - HALF_PI), TWO_PI);
    // Same as ^, but shifted PI becuase psi_0 has been shifted PI (to face the opposite center)
    double beta_2 = mod((M_PI + psi_0) - alpha - (end.psi + HALF_PI), TWO_PI);

    // pythagorean theroem to calculate distance off of known right trangle using intercenter/radius
    double straight_distance = 2 * std::sqrt(std::pow(half_intercenter_distance, 2) - std::pow(_radius, 2));
    double total_distance = _radius * (beta_0 + beta_2) + straight_distance;

    return RRTOption(total_distance, DubinsPath(beta_0, -beta_2, straight_distance), true);
}

RRTOption Dubins::rsl(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
{
    Eigen::Vector2d half_displacement = halfDisplacement(center_2, center_0);
    double psi_0 = std::atan2(half_displacement.y(), half_displacement.x());
    double half_intercenter_distance = half_displacement.norm();

    if (half_intercenter_distance < _radius)
    {
        return RRTOption(std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), true);
    }

    // angle between intercenter displacement vector AND vector orthogonal to the "leave circle" vector
    double alpha = std::acos(_radius / half_intercenter_distance);
    // (STARTING_ANGLE) - (ANGLE TO TERMINAL) [assuming everything is relative to horizontal]
    // i.e. (start ==> normal to circle) - (angle to connect center + angle between center an "leave circle" point)
    double beta_0 = mod((start.psi + HALF_PI) - (psi_0 + alpha), TWO_PI);
    // Same as ^, but shifted PI becuase psi_0 has been shifted PI (to face the opposite center)
    double beta_2 = mod((end.psi - HALF_PI) - (alpha + psi_0 + M_PI), TWO_PI);

    // pythagorean theroem to calculate distance off of known right trangle using intercenter/radius
    double straight_distance = 2 * std::sqrt(std::pow(half_intercenter_distance, 2) - std::pow(_radius, 2));
    double total_distance = _radius * (beta_0 + beta_2) + straight_distance;

    return RRTOption(total_distance, DubinsPath(-beta_0, beta_2, straight_distance), true);
}

RRTOption Dubins::lrl(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
{
    double intercenter_distance = distanceBetween(center_0, center_2);
    Eigen::Vector2d half_displacement = halfDisplacement(center_2, center_0);
    double psi_0 = std::atan2(half_displacement.y(), half_displacement.x());

    if (intercenter_distance < 2 * _radius && intercenter_distance > 4 * _radius)
    {
        return RRTOption(std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), false);
    }

    // angle formed by the connection of the radii (the isosoles triange) (using similar triangles)
    // 2 * (sin of the triangle made by splitting the isocoles triangle in half (a similar triangle twice as large))
    //                                                  [half of gamma (splitting the right triangle in two)]
    double gamma = 2 * std::asin(intercenter_distance / (4 * _radius));
    // bottom two angles of isocoles (180 - gamma) = 2 * theta
    double theta = (M_PI - gamma) / 2;
    // (ANGLE REQUIRED TO GET FROM START TO psi_0) + (THETA)
    double beta_0 = mod(psi_0 - (start.psi - HALF_PI) + theta, TWO_PI);
    // (ANGLE FROM psi_0 --> END) + (THETA) [psi_0 now needs to point towards center_0]
    double beta_2 = mod((end.psi - HALF_PI) - (psi_0 + M_PI) + theta, TWO_PI);

    // beta_1 ==> (2PI - gamma) is positive angle that the plane flys through
    double beta_1 = TWO_PI - gamma;
    double total_distance = (beta_1 + beta_0 + beta_2) * _radius;
    return RRTOption(total_distance, DubinsPath(beta_0, beta_2, -beta_1), false);
}

RRTOption Dubins::rlr(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
{
    double intercenter_distance = distanceBetween(center_0, center_2);
    Eigen::Vector2d half_displacement = halfDisplacement(center_2, center_0);
    double psi_0 = std::atan2(half_displacement.y(), half_displacement.x());

    if (intercenter_distance < 2 * _radius && intercenter_distance > 4 * _radius)
    {
        return RRTOption(std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), false);
    }

    // angle formed by the connection of the radii (the isosoles triange) (using similar triangles)
    // 2 * (sin of the triangle made by splitting the isocoles triangle in half (a similar triangle twice as large))
    //                                                  [half of gamma (splitting the right triangle in two)]
    double gamma = 2 * std::asin(intercenter_distance / (4 * _radius));
    double theta = (M_PI - gamma) / 2;

    // same as lrl, except its a negative angle because its a right turn
    // (ANGLE REQUIRED TO GET FROM START TO psi_0) + (THETA)
    double beta_0 = mod((start.psi + HALF_PI) - psi_0 + theta, TWO_PI);
    // (ANGLE FROM psi_0 --> END) + (THETA) [psi_0 now needs to point towards center_0]
    double beta_2 = mod((psi_0 + HALF_PI) - end.psi + theta, TWO_PI);

    // beta_1 ==> (2PI - gamma) is positive angle that the plane flys through
    double beta_1 = TWO_PI - gamma;
    double total_distance = (beta_1 + beta_0 + beta_2) * _radius;
    return RRTOption(total_distance, DubinsPath(-beta_0, -beta_2, beta_1), false);
}

std::vector<RRTOption> Dubins::allOptions(const XYZCoord &start, const XYZCoord &end, bool sort)
{
    Eigen::Vector2d center_0_left = findCenter(start, 'L');
    Eigen::Vector2d center_0_right = findCenter(start, 'R');
    Eigen::Vector2d center_2_left = findCenter(end, 'L');
    Eigen::Vector2d center_2_right = findCenter(end, 'R');

    std::vector<RRTOption> options = {
        lsl(start, end, center_0_left, center_2_left),
        rsr(start, end, center_0_right, center_2_right),
        rsl(start, end, center_0_right, center_2_left),
        lsr(start, end, center_0_left, center_2_right),
        rlr(start, end, center_0_right, center_2_right),
        lrl(start, end, center_0_left, center_2_left)};

    if (sort)
    {
        std::sort(options.begin(), options.end(), compareRRTOptionLength);
    }

    return options;
}

std::vector<Eigen::Vector2d> Dubins::dubinsPath(const XYZCoord &start, const XYZCoord &end)
{
    std::vector<RRTOption> options = allOptions(start, end);
    RRTOption optimal_option = *std::min_element(options.begin(), options.end(), compareRRTOptionLength);
    return generatePoints(start, end, optimal_option.dubins_path, optimal_option.has_straight);
}

double _radius;
double _point_separation;

