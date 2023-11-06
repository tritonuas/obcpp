#ifndef PATHING_DUBINS_HPP_
#define PATHING_DUBINS_HPP_

#include <math.h>
#include <limits>

#include "../utilities/datatypes.hpp"

#include "Eigen"

const double TWO_PI_RAD = 2 * M_PI;
const double HALF_PI_RAD = HALF_PI_RAD;

struct DubinsPath
{
    DubinsPath(double beta_0, double beta_2, double straight_dist)
        : beta_0(beta_0), beta_2(beta_2), straight_dist(straight_dist) {}

    double beta_0;        // total angle turned in first_turn     radians
    double beta_2;        // total angle turned in second_turn    radians
    double straight_dist; // distance that the path moves in a straightaway
};

struct RRTOption
{
    RRTOption(double length, DubinsPath dubins_path, bool has_straight)
        : length(length), dubins_path(dubins_path), has_straight(has_straight) {}

    double length;          // [TODO]
    DubinsPath dubins_path; // [TODO]
    bool has_straight;      // if this option has a straight path or not
};

/**
 *  Reproduction of np.sign() function as used in the older obc
 *  from https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
 *
 *  @param number   ==> any double
 *  @return         ==> -1 IF number < 0
 *                       1 IF number > 0
 *                       0 IF number == 0
 */
template <typename T>
int sign(T val)
{
    return (T(0) < val) - (val < T(0));
}

/**
 *  @param vector   ==> 2-vector
 *  @return         ==> orthogonal 2-vector to @param vector (always a 90 degree rotation counter-clockwise)
 *  @see https://mathworld.wolfram.com/PerpendicularVector.html
 */
Eigen::Vector2d findOrthogonalVector2D(const Eigen::Vector2d &vector)
{
    return Eigen::Vector2d(-vector[1], vector[0]);
}

/**
 *   @param vector1  ==> 2-vector
 *   @param vector2  ==> 2-vector
 *   @return         ==> the magnitude of the displacement vector between the two vectors
 */
double distanceBetween(const Eigen::Vector2d &vector1, const Eigen::Vector2d &vector2)
{
    return (vector1 - vector2).norm();
}

/*
 *
 *   [TODO] - write desc.
 *
 */
Eigen::Vector2d midpoint(const Eigen::Vector2d &vector1, const Eigen::Vector2d &vector2)
{
    return (vector1 + vector2) / 2;
}

class Dubins
{
    /*
        TODO:
            - copy the python code from the other obc repo
    */
public:
    Dubins(double radius, double point_separation)
        : _radius(radius), _point_separation(point_separation)
    {
        assert(radius > 0);
        assert(point_separation > 0);
    }

    /**
     *   @param  point   ==> current position of the plane (vector) with psi in radians
     *   @param  side    ==> whether the plane is planning to turn left (L) or right (R)
     *   @return         ==> center of a turning circle
     */
    Eigen::Vector2d findCenter(const XYZCoord &point, char side)
    {
        assert(side == 'L' || side == 'R');

        // creates a right angle between the XYZCoord vector towards the center
        // left is 90 deg CCW, right is 90 deg CW
        double angle = point.psi + (side == 'L' ? HALF_PI_RAD : -HALF_PI_RAD);

        // creates the vector offset from the existing position
        return Eigen::Vector2d(point.x + (std::cos(angle) * this->_radius),
                               point.y + (std::sin(angle) * this->_radius));
    }

    /**
     *   @param starting_point   ==> position vector of the plane (only psi is used to ascertain direction)
     *   @param beta             ==> angle of the turn (positive is left/ccw)
     *   @param center           ==> center of the circle
     *   @param path_length      ==> the arc-length along the circle
     *   @returns                ==> point along circle path
     */
    Eigen::Vector2d circleArc(const XYZCoord &starting_point, double beta, const Eigen::Vector2d &center, double path_length)
    {
        // forward angle + [(displacement angle to the right) * -1 IF angle is negative ELSE 1]
        double angle = starting_point.psi + ((path_length / this->_radius) - HALF_PI_RAD) * sign(beta);
        // unit vector encoding angle information
        Eigen::Vector2d angle_vector(std::cos(angle), std::sin(angle));
        return center + (angle_vector * this->_radius);
    }

    /**
     *  Generates points for the paths that contain a straight section
     *  [LSL, LSR, RSR, RSL]
     *
     *  @param start    ==> current "vector" of the plane
     *  @param end      ==> final/desired "vector" of the path
     *  @param path     ==> Some given dubins's path parameters
     *  @return         ==> a list of vectors along the entire path
     */
    std::vector<Eigen::Vector2d> generatePointsStraight(const XYZCoord &start, const XYZCoord &end, const DubinsPath &path)
    {
        // the arclength of both curved sections + straight section
        double total_distance = this->_radius * (std::abs(path.beta_0) + std::abs(path.beta_2)) + path.straight_dist;

        Eigen::Vector2d center_0 = this->findCenter(start, (path.beta_0 > 0) ? 'L' : 'R');
        Eigen::Vector2d center_2 = this->findCenter(end, (path.beta_2 > 0) ? 'L' : 'R');

        Eigen::Vector2d initial_point(0, 0); // start of the straight secton

        Eigen::Vector2d final_point(0, 0); // end of the straight section

        // finds starting and ending points of the _straight section_
        if (std::abs(path.beta_0) > 0)
        {
            // The total angle that the path is planning to turn along turn 1 (referece to center of the circle)
            // (start direction) + (*FROM CENTER OF CIRCLE, the angle traveled along curve * whether the plane is turning left/right)
            double angle = start.psi + (std::abs(path.beta_0) - HALF_PI_RAD) * sign(path.beta_0);
            initial_point = center_0 + this->_radius * Eigen::Vector2d(std::cos(angle), std::sin(angle));
        }
        else
        {
            initial_point = Eigen::Vector2d(start.x, start.y);
        }

        if (std::abs(path.beta_2) > 0)
        {
            // negative sign before beta_2 is because the path comes in through the back of the end vector
            double angle = end.psi + (-std::abs(path.beta_2) - HALF_PI_RAD) * sign(path.beta_2);
            final_point = center_2 + this->_radius * Eigen::Vector2d(std::cos(angle), std::sin(angle));
        }
        else
        {
            final_point = Eigen::Vector2d(end.x, end.y);
        }

        double distance_straight = distanceBetween(initial_point, final_point);

        //  generates the points for the entire curve.
        std::vector<Eigen::Vector2d> points_list;
        for (int distance = 0; distance < total_distance; distance += this->_point_separation)
        {
            if (distance < std::abs(path.beta_0) * this->_radius)
            { // First turn
                points_list.push_back(this->circleArc(start, path.beta_0, center_0, distance));
            }
            else if (distance > total_distance - std::abs(path.beta_2) * this->_radius)
            { // Last turn
                points_list.push_back(this->circleArc(end, path.beta_2, center_2, distance));
            }
            else
            { // Straignt Section
                // coefficient is the ratio of the straight distance that has been traversed.
                // (current_distance_traved - (ENTIRE_LENGTH_OF_CURVED_PATH)) / length_of_the_straight_path
                double coefficient = (distance - (std::abs(path.beta_0) * this->_radius)) / distance_straight;
                // convex linear combination to find the vector along the straight path between the initial and final point
                points_list.push_back(coefficient * final_point + (1 - coefficient) * initial_point);
            }
        }

        points_list.push_back(Eigen::Vector2d(end.x, end.y));
        return points_list;
    }

    /**
     *  Generates points for the paths that only contain curved sections
     *  [RLR, LRL]
     *
     *  @param start    ==> current "vector" of the plane
     *  @param end      ==> final/desired "vector" of the path
     *  @param path     ==> Some given dubins's path parameters
     *  @return         ==> a list of vectors along the entire path
     */
    std::vector<Eigen::Vector2d> generatePointsCurve(const XYZCoord &start, const XYZCoord &end, const DubinsPath &path)
    {
        // the arclength of all paths
        double total_distance = this->_radius * (std::abs(path.beta_2) + std::abs(path.beta_0) + std::abs(path.straight_dist));

        Eigen::Vector2d center_0 = findCenter(start, (path.beta_0 > 0) ? 'L' : 'R');
        Eigen::Vector2d center_2 = findCenter(end, (path.beta_2 > 0) ? 'L' : 'R');

        double intercenter = distanceBetween(center_0, center_2);

        // should be law of cosines or some shit
        Eigen::Vector2d center_1 = midpoint(center_0, center_2) // midpoint between the centers
                                   + sign(path.beta_0) * sqrt(  // hypotnuse - 2r (distance between center0/2 and center 1) | a - intercenter / 2 (half of distance between center 0 and center 2)
                                                             4 * this->_radius * this->_radius - (intercenter / 2) * (intercenter / 2)) *
                                         findOrthogonalVector2D((center_2 - center_0) / intercenter); // unit vector (direction vector) orthogonal to the displacement vector between the two centers

        // angle from center 1 pointing towards center 0 parallel to the displacement vector
        // angle of the displacement vector relative to x+ (horizontal) MINUS pi (rotated 180 clockwise)
        double psi_0 = std::atan2(center_1[1] - center_0[1], center_1[0] - center_0[0]) - M_PI;

        // now to generate the actual points
        std::vector<Eigen::Vector2d> points_list;
        for (double distance = 0; distance < total_distance; distance += _point_separation)
        {
            if (distance < std::abs(path.beta_0) * this->_radius)
            { // First turn
                points_list.push_back(this->circleArc(start, path.beta_0, center_0, distance));
            }
            else if (distance > total_distance - std::abs(path.beta_2) * this->_radius)
            { // Last turn
                points_list.push_back(this->circleArc(end, path.beta_2, center_2, distance - total_distance));
            }
            else
            { // Middle Turn
                // angle relative to center 1 pointing around the curve
                // from left + (angular distance for center 1)
                // left pointer + (+LEFT/-RIGHT) (total angular distance - angular distance for turn 1)
                double angle = psi_0 - sign(path.beta_0) * (distance / this->_radius - std::abs(path.beta_0));
                Eigen::Vector2d vect(std::cos(angle), std::sin(angle));
                points_list.push_back(center_1 + this->_radius * vect);
            }
        }

        return points_list;
    }

    std::vector<Eigen::Vector2d> generatePoints(const XYZCoord &start, const XYZCoord &end, const DubinsPath &path, bool has_straight)
    {
        if (has_straight)
        {
            return this->generatePointsStraight(start, end, path);
        }
        else
        {
            return this->generatePointsCurve(start, end, path);
        }
    }

    /**
     *  First computes the poisition of the centers of the turns, and then uses
        the fact that the vector defined by the distance between the centers
        gives the direction and distance of the straight segment.
    */
    RRTOption lsl(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
    {
        double straight_distance = distanceBetween(center_0, center_2);

        // angle relative to horizontal
        double alpha = std::atan2(center_2[1] - center_0[1], center_2[0] - center_0[0]);

        // difference in angle on the interval [-2pi, 2pi]
        double beta_0 = std::fmod(alpha - start.psi, TWO_PI_RAD);
        double beta_2 = std::fmod(end.psi - alpha, TWO_PI_RAD);

        double total_distance = this->_radius * (beta_0 + beta_2) + straight_distance;

        return RRTOption(total_distance, DubinsPath(beta_0, beta_2, straight_distance), true);
    }

    /**
     * First computes the poisition of the centers of the turns, and then uses
     *    the fact that the vector defined by the distance between the centers
     *    gives the direction and distance of the straight segment.
     *   [TODO] - write documentation
     */
    RRTOption rsr(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
    {
        double straight_distance = distanceBetween(center_0, center_2);

        // angle relative to horizontal
        double alpha = std::atan2(center_2[1] - center_0[1], center_2[0] - center_0[0]);

        double beta_0 = std::fmod(-alpha + start.psi, TWO_PI_RAD);
        double beta_2 = std::fmod(-end.psi + alpha, TWO_PI_RAD);

        double total_distance = this->_radius * (beta_2 + beta_0) + straight_distance;

        return RRTOption(total_distance, DubinsPath(-beta_0, -beta_2, straight_distance), true);
    }

    /**
     *  Because of the change in turn direction, it is a little more complex to
        compute than in the RSR or LSL cases. First computes the position of
        the centers of the turns, and then uses the rectangle triangle defined
        by the point between the two circles, the center point of one circle
        and the tangeancy point of this circle to compute the straight segment
        distance.
     */
    RRTOption rsl(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
    {
        Eigen::Vector2d mid_point = midpoint(center_0, center_2);
        double psi_a = std::atan2(mid_point[1], mid_point[0]);
        double half_intercenter_length = mid_point.norm();

        if (half_intercenter_length < this->_radius)
        {
            return RRTOption(std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), true);
        }

        double alpha = std::acos(this->_radius / half_intercenter_length);
        double beta_0 = std::fmod(-(psi_a + alpha - start.psi - HALF_PI_RAD), TWO_PI_RAD);
        // is is better to simplify?
        double beta_2 = std::fmod(M_PI + end.psi - HALF_PI_RAD - alpha - psi_a, TWO_PI_RAD);

        double straight_distance = 2 * std::pow(std::pow(half_intercenter_length, 2) - std::pow(this->_radius, 2), 0.5);
        double total_distance = this->_radius * (beta_0 + beta_2) + straight_distance;

        return RRTOption(total_distance, DubinsPath(-beta_0, beta_2, straight_distance), true);
    }

    /**
     *  Because of the change in turn direction, it is a little more complex to
        compute than in the RSR or LSL cases. First computes the poisition of
        the centers of the turns, and then uses the rectangle triangle defined
        by the point between the two circles, the center point of one circle
        and the tangeancy point of this circle to compute the straight segment
        distance.
     */
    RRTOption lsr(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
    {
        Eigen::Vector2d mid_point = midpoint(center_0, center_2);
        double psi_a = std::atan2(mid_point[1], mid_point[0]);
        double half_intercenter_length = mid_point.norm();

        if (half_intercenter_length < this->_radius)
        {
            return RRTOption(std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), true);
        }

        double alpha = std::acos(this->_radius / half_intercenter_length);
        double beta_0 = std::fmod(psi_a - alpha - start.psi + HALF_PI_RAD, TWO_PI_RAD);
        // is is better to simplify?
        double beta_2 = std::fmod(HALF_PI_RAD - end.psi - alpha + psi_a, TWO_PI_RAD);

        double straight_distance = 2 * std::pow(std::pow(half_intercenter_length, 2) - std::pow(this->_radius, 2), 0.5);
        double total_distance = this->_radius * (beta_0 + beta_2) + straight_distance;

        return RRTOption(total_distance, DubinsPath(beta_0, -beta_2, straight_distance), true);
    }
        
    /**
     *  Using the isoceles triangle made by the centers of the three circles,
        computes the required angles.
     */
    RRTOption lrl(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
    {
        double intercenter_distance = distanceBetween(center_0, center_2);
        Eigen::Vector2d mid_point = midpoint(center_0, center_2);
        double psi_a = std::atan2(mid_point[1], mid_point[0]);

        // better to simplify?
        if (2 * this->_radius < intercenter_distance && intercenter_distance > 4 * this->_radius)
        {
            return RRTOption(std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), false);
        }

        double gamma = 2 * std::asin(intercenter_distance / (4 * this->_radius));
        double beta_0 = std::fmod(psi_a - start.psi + HALF_PI_RAD + (M_PI - gamma) / 2, TWO_PI_RAD);
        // why called beta_1
        double beta_1 = std::fmod(-psi_a + HALF_PI_RAD + end.psi + (M_PI - gamma) / 2, TWO_PI_RAD);
        double total_distance = (TWO_PI_RAD - gamma + std::abs(beta_0) + std::abs(beta_1)) * this->_radius;
        return RRTOption(total_distance, DubinsPath(beta_0, beta_1, TWO_PI_RAD - gamma), false);
    }

    /**
     *  Using the isoceles triangle made by the centers of the three circles,
        computes the required angles.
     */
    RRTOption rlr(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2)
    {
        double intercenter_distance = distanceBetween(center_0, center_2);
        Eigen::Vector2d mid_point = midpoint(center_0, center_2);
        double psi_a = std::atan2(mid_point[1], mid_point[0]);

        // better to simplify?
        if (2 * this->_radius < intercenter_distance && intercenter_distance > 4 * this->_radius)
        {
            return RRTOption(std::numeric_limits<double>::infinity(), DubinsPath(0, 0, 0), false);
        }

        double gamma = 2 * std::asin(intercenter_distance / (4 * this->_radius));
        double beta_0 = std::fmod(psi_a - start.psi + HALF_PI_RAD + (M_PI - gamma) / 2, TWO_PI_RAD);
        // why called beta_1
        double beta_1 = std::fmod(-psi_a + HALF_PI_RAD + end.psi + (M_PI - gamma) / 2, TWO_PI_RAD);
        double total_distance = (TWO_PI_RAD - gamma + std::abs(beta_0) + std::abs(beta_1)) * this->_radius;
        return RRTOption(total_distance, DubinsPath(beta_0, beta_1, TWO_PI_RAD - gamma), false);
    }

    /**
     * Compute all the possible Dubin's path and returns them.

        Returns in the form of a list of tuples representing each option: (path_length,
        dubins_path, straight).
     * 
    */
    std::vector<RRTOption> allOptions(const XYZCoord &start, const XYZCoord &end, bool sort=false) {
        Eigen::Vector2d center_0_left = this->findCenter(start, 'L');
        Eigen::Vector2d center_0_right = this->findCenter(start, 'R');
        Eigen::Vector2d center_2_left = this->findCenter(end, 'L');
        Eigen::Vector2d center_2_right = this->findCenter(end, 'R');

        std::vector<RRTOption> options = {
            this->lsl(start, end, center_0_left, center_2_left),
            this->rsr(start, end, center_0_right, center_2_right),
            this->rsl(start, end, center_0_right, center_2_left),
            this->lsr(start, end, center_0_left, center_2_right),
            this->rlr(start, end, center_0_right, center_2_right),
            this->rlr(start, end, center_0_left, center_2_left)
        };

        if (sort)
        {
            std::sort(options.begin(), options.end(), [](RRTOption a, RRTOption b)
            {
                return a.length > b.length;
            });
        }
        return options;
    }

    /**
     * Compute all the possible Dubin's path.

        Returns sequence of points representing the shortest option.
    */
    std::vector<Eigen::Vector2d> dubinsPath(const XYZCoord &start, const XYZCoord &end) {
        std::vector<RRTOption> options = this->allOptions(start, end);
        // too lazy to find a proper function to find min for vectors (as of now)
        RRTOption optimal_option = options[0];

        for (int i = 1; i < options.size(); i++) {
            if (optimal_option.length > options[i].length) {
                optimal_option = options[i];
            }
        } 

        return this->generatePoints(start, end, optimal_option.dubins_path, optimal_option.has_straight);
    }

private:
    double _radius;
    double _point_separation;
};

#endif // PATHING_DUBINS_HPP_
