#ifndef PATHING_DUBINS_HPP_
#define PATHING_DUBINS_HPP_

#include <cmath>
#include <limits>

#include "Eigen"

#include "../utilities/datatypes.hpp"

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

const double TWO_PI = 2 * M_PI;
const double HALF_PI = M_PI / 2;

struct DubinsPath
{
    DubinsPath(double beta_0, double beta_2, double straight_dist)
        : beta_0(beta_0), beta_2(beta_2), straight_dist(straight_dist) {}

    double beta_0;        // total angle turned in first_turn     radians
    double beta_2;        // total angle turned in last_turn    radians
    double straight_dist; // distance that the path moves in a straightaway
                          // IF [LRL, RLR], beta_1 ==> angle for middle_turn
                          //                !!! beta_1 only used for total_distance, see Dubins::lrl() or Dubins::rlr()
};

struct RRTOption
{
    RRTOption(double length, DubinsPath dubins_path, bool has_straight)
        : length(length), dubins_path(dubins_path), has_straight(has_straight) {}

    double length;          // the total length of the path
    DubinsPath dubins_path; // parameters of DubinsPath
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
int sign(T val);

/** ((a % n) + n) % n
 *  a + n * [a / n] where [] is the floor function (i.e. integer division)
 *  Mimics mod operator as used in python
 *
 * @param a    ==> the dividend
 * @param n    ==> the divisor
 * @returns    ==> positive number reflecting the remainder
 * @see / from ==> https://stackoverflow.com/questions/1907565/c-and-python-different-behaviour-of-the-modulo-operation
 */
double mod(double a, double n);


/**
 *  For sorting function, a min sort bsed on path length
 *  i.e shortest path better
 *
 *  @param first    ==> first option
 *  @param second   ==> second option
 *  @return         ==> true if @param first has a smaller length
 */
bool compareRRTOptionLength(const RRTOption &first, const RRTOption &second);


/**
 *  Finds a orthogonal 2-vector to the 2-vector inputted
 *
 *  @param vector   ==> 2-vector
 *  @return         ==> orthogonal 2-vector to @param vector (always a 90 degree rotation counter-clockwise)
 *  @see    https://mathworld.wolfram.com/PerpendicularVector.html
 */
Eigen::Vector2d findOrthogonalVector2D(const Eigen::Vector2d &vector);

/**
 *   finds the distance between two 2-vectors
 *
 *   @param vector1  ==> 2-vector
 *   @param vector2  ==> 2-vector
 *   @return         ==> the magnitude of the displacement vector between the two vectors
 */
double distanceBetween(const Eigen::Vector2d &vector1, const Eigen::Vector2d &vector2);

/**
 *  returns half of the displacement vector from v2 to v1
 *
 *  @param  vector1 ==> 2-vector
 *  @param  vector2 ==> 2-vector
 *  @return         ==> the displacment vector from @param vector2 to @param vector1 that
 *                      terminates half way (i.e. half the magnitude)
 */
Eigen::Vector2d halfDisplacement(const Eigen::Vector2d &vector1, const Eigen::Vector2d &vector2);


class Dubins
{
public:
    Dubins(double radius, double point_separation);


    /**
     *   Finds the center of a given turn orignating at a vector turning left or right
     *   Assumes the path may be done perfectly circularly
     *
     *   @param  point   ==> current position of the plane (vector) with psi in radians
     *   @param  side    ==> whether the plane is planning to turn left (L) or right (R)
     *   @return         ==> center of a turning circle
     */
    Eigen::Vector2d findCenter(const XYZCoord &point, char side);

    /**
     *   Finds a point (vector) along a curved path given a distance already traveled
     *
     *   @param starting_point   ==> position vector of the plane (only psi is used to ascertain direction)
     *   @param beta             ==> angle of the turn (positive is left/ccw)
     *   @param center           ==> center of the circle
     *   @param path_length      ==> the arc-length along the circle
     *   @returns                ==> point along circle path
     */
    Eigen::Vector2d circleArc(const XYZCoord &starting_point, double beta, const Eigen::Vector2d &center, double path_length);
    /**
     *  Generates points for the paths that contain a straight section
     *  [LSL, LSR, RSR, RSL]
     *
     *  @param start    ==> current "vector" of the plane
     *  @param end      ==> final/desired "vector" of the path
     *  @param path     ==> Some given dubins's path parameters
     *  @return         ==> a list of vectors along the entire path
     */
    std::vector<Eigen::Vector2d> generatePointsStraight(const XYZCoord &start, const XYZCoord &end, const DubinsPath &path);

    /**
     *  Generates points for the paths that only contain curved sections
     *  [RLR, LRL]
     *
     *  @param start    ==> current "vector" of the plane
     *  @param end      ==> final/desired "vector" of the path
     *  @param path     ==> Some given dubins's path parameters
     *  @return         ==> a list of vectors along the entire path
     */
    std::vector<Eigen::Vector2d> generatePointsCurve(const XYZCoord &start, const XYZCoord &end, const DubinsPath &path);

    /**
     *  Abstraction for generating points (curved/straight)
     *
     *  @param start        ==> vector at start position
     *  @param end          ==> vector at end position
     *  @param path         ==> DubinsPath encoding turning and straight information
     *  @param has_straigt  ==> whether the given DubinsPath has a straight section or not
     *  @return             ==> a list of points that represent the shortest dubin's path from start to end
     */
    std::vector<Eigen::Vector2d> generatePoints(const XYZCoord &start, const XYZCoord &end, const DubinsPath &path, bool has_straight); 

    /**
     *  First, the straight distance (it turns out) is equal to the
     *  distance between the two centers
     *  Next it calculates the turning angle (positive [CCW] turn from start to end vector)
     *
     *  @param start    ==> vector at start position
     *  @param end      ==> vector at end position
     *  @param center_0 ==> the center of the first turn
     *  @param center_2 ==> the center of the last turn
     *  @return             RRTOption detailing the parameters of the path
     *                      - total distance
     *                      - DubinsPath
     *                          - turning angle 1
     *                          - turning angle 2
     *                          - straight_distance
     *                      - if the path has a straight section
     */
    RRTOption lsl(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2);

    /**
     *  First, the straight distance (it turns out) is equal to the
     *  distance between the two centers
     *  Next it calculates the turning angle (negative [CW] turn from start to end vector)
     *
     *  @param start    ==> vector at start position
     *  @param end      ==> vector at end position
     *  @param center_0 ==> the center of the first turn
     *  @param center_2 ==> the center of the last turn
     *  @return             RRTOption detailing the parameters of the path
     *                      - total distance
     *                      - DubinsPath
     *                          - turning angle 1
     *                          - turning angle 2
     *                          - straight_distance
     *                      - if the path has a straight section
     */
    RRTOption rsr(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2);

    /**
     *  Because of the change in turn direction, it is a little more complex to
     *  compute than in the RSR or LSL cases.
     *
     *  The tangent line between both turns turns out to bisect the intercenter.
     *  With that knowledge, it is possible to compute both the length and the angle of the
     *  tangent line ==> then allowing the calculation for the proper turning angles.
     *
     *  @param start    ==> vector at start position
     *  @param end      ==> vector at end position
     *  @param center_0 ==> the center of the first turn
     *  @param center_2 ==> the center of the last turn
     *  @return             RRTOption detailing the parameters of the path
     *                      - total distance
     *                      - DubinsPath
     *                          - turning angle 1
     *                          - turning angle 2
     *                          - straight_distance
     *                      - if the path has a straight section
     */
    RRTOption lsr(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2);

    /**
     *  Because of the change in turn direction, it is a little more complex to
     *  compute than in the RSR or LSL cases.
     *
     *  The tangent line between both turns turns out to bisect the intercenter.
     *  With that knowledge, it is possible to compute both the length and the angle of the
     *  tangent line ==> then allowing the calculation for the proper turning angles.
     *
     *  @param start    ==> vector at start position
     *  @param end      ==> vector at end position
     *  @param center_0 ==> the center of the first turn
     *  @param center_2 ==> the center of the last turn
     *  @return             RRTOption detailing the parameters of the path
     *                      - total distance
     *                      - DubinsPath
     *                          - turning angle 1
     *                          - turning angle 2
     *                          - straight_distance
     *                      - if the path has a straight section
     */
    RRTOption rsl(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2);

    /**
     *  Using the isoceles triangle made by the centers of the three circles,
     *  computes the required angles.
     *
     *  @param start    ==> vector at start position
     *  @param end      ==> vector at end position
     *  @param center_0 ==> the center of the first turn
     *  @param center_2 ==> the center of the last turn
     *  @return             RRTOption detailing the parameters of the path
     *                      - total distance
     *                      - DubinsPath
     *                          - turning angle 1
     *                          - turning angle 2
     *                          - straight_distance
     *                      - if the path has a straight section
     */
    RRTOption lrl(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2);

    /**
     *  Using the isoceles triangle made by the centers of the three circles,
     *  computes the required angles.
     *
     *  @param start    ==> vector at start position
     *  @param end      ==> vector at end position
     *  @param center_0 ==> the center of the first turn
     *  @param center_2 ==> the center of the last turn
     *  @return             RRTOption detailing the parameters of the path
     *                      - total distance
     *                      - DubinsPath
     *                          - turning angle 1
     *                          - turning angle 2
     *                          - straight_distance
     *                      - if the path has a straight section
     */
    RRTOption rlr(const XYZCoord &start, const XYZCoord &end, const Eigen::Vector2d &center_0, const Eigen::Vector2d &center_2);

    /**
     * Compute all the possible Dubin's path and returns a list
     *  containing RRTOption(s) with path data.
     *
     *  @param start    ==> vector at start position
     *  @param end      ==> vector at end position
     *  @param sort     ==> whether the method sorts the resulting vector DEFALT-->FALSE (searching is faster)
     *  @return         ==> list containing all the RRTOptions from the path generation
     */
    std::vector<RRTOption> allOptions(const XYZCoord &start, const XYZCoord &end, bool sort);

    /**
     * Compute all the possible Dubin's path(s) and
     * Returns sequence of points representing the shortest option.
     *
     *  @param start    ==> vector at start position
     *  @param end      ==> vector at end position
     *  @return         ==> the points for the most optimal path from @param start to @param end
     */
    std::vector<Eigen::Vector2d> dubinsPath(const XYZCoord &start, const XYZCoord &end);

private:
    double _radius;
    double _point_separation;
};

#endif // PATHING_DUBINS_HPP_
