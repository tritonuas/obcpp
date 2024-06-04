#ifndef INCLUDE_PATHING_DUBINS_HPP_
#define INCLUDE_PATHING_DUBINS_HPP_

#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

#include "utilities/datatypes.hpp"

struct DubinsPath {
    DubinsPath(double beta_0, double beta_2, double straight_dist)
        : beta_0(beta_0), beta_2(beta_2), straight_dist(straight_dist) {}

    double beta_0;         // total angle turned in first_turn     radians
    double beta_2;         // total angle turned in last_turn    radians
    double straight_dist;  // distance that the path moves in a straightaway
                           // IF [LRL, RLR], beta_1 ==> angle for middle_turn
                           // !!! beta_1 only used for total_distance,
                           // see Dubins::lrl() or Dubins::rlr()
};

struct RRTOption {
    RRTOption(double length, DubinsPath dubins_path, bool has_straight)
        : length(length), dubins_path(dubins_path), has_straight(has_straight) {}

    double length;           // the total length of the path
    DubinsPath dubins_path;  // parameters of DubinsPath
    bool has_straight;       // if this option has a straight path or not
};

/**
 *  Reproduction of np.sign() function as used in the older obc
 *  from
 * https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
 *
 *  @param number   ==> any double
 *  @return         ==> -1 IF number < 0
 *                       1 IF number > 0
 *                       0 IF number == 0
 */
template <typename T>
int sign(T number);

/** ((a % n) + n) % n
 *  a + n * [a / n] where [] is the floor function (i.e. integer division)
 *  Mimics mod operator as used in python
 *
 * @param dividend    ==> the dividend
 * @param divisor    ==> the divisor
 * @returns    ==> positive number reflecting the remainder
 * @see / from ==>
 * https://stackoverflow.com/questions/1907565/c-and-python-different-behaviour-of-the-modulo-operation
 */
double mod(double dividend, double divisor);

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
 *  @return         ==> orthogonal 2-vector to @param vector (always a 90 degree rotation
 * counter-clockwise)
 *  @see    https://mathworld.wolfram.com/PerpendicularVector.html
 */
XYZCoord findOrthogonalVector2D(const XYZCoord &vector);

/**
 *  returns half of the displacement vector from v2 to v1
 *
 *  @param  vector1 ==> 2-vector
 *  @param  vector2 ==> 2-vector
 *  @return         ==> the displacment vector from @param vector2 to
 *                      @param vector1 that terminates half way
 *                      (i.e. half the magnitude)
 */
XYZCoord halfDisplacement(const XYZCoord &vector1, const XYZCoord &vector2);

class Dubins {
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
    XYZCoord findCenter(const RRTPoint &point, char side) const;

    /**
     *   Finds a point (vector) along a curved path given a distance
     *      already traveled
     *
     *   @param starting_point   ==> position vector of the plane (only psi is
     *                                  used to ascertain direction)
     *   @param beta             ==> angle of the turn (positive is left/ccw)
     *   @param center           ==> center of the circle
     *   @param path_length      ==> the arc-length along the circle
     *   @returns                ==> point along circle path
     */
    XYZCoord circleArc(const RRTPoint &starting_point, double beta, const XYZCoord &center,
                     double path_length) const;
    /**
     *  Generates points for the paths that contain a straight section
     *  [LSL, LSR, RSR, RSL]
     *
     *  @param start    ==> current "vector" of the plane
     *  @param end      ==> final/desired "vector" of the path
     *  @param path     ==> Some given dubins's path parameters
     *  @return         ==> a list of vectors along the entire path
     */
    std::vector<XYZCoord> generatePointsStraight(const RRTPoint &start, const RRTPoint &end,
                                               const DubinsPath &path) const;

    /**
     *  Generates points for the paths that only contain curved sections
     *  [RLR, LRL]
     *
     *  @param start    ==> current "vector" of the plane
     *  @param end      ==> final/desired "vector" of the path
     *  @param path     ==> Some given dubins's path parameters
     *  @return         ==> a list of vectors along the entire path
     */
    std::vector<XYZCoord> generatePointsCurve(const RRTPoint &start, const RRTPoint &end,
                                            const DubinsPath &path) const;

    /**
     *  Abstraction for generating points (curved/straight)
     *
     *  @param start        ==> vector at start position
     *  @param end          ==> vector at end position
     *  @param path         ==> DubinsPath encoding turning and straight information
     *  @param has_straigt  ==> whether the given DubinsPath has a straight section or not
     *  @return             ==> a list of points that represent the shortest
     *                          dubin's path from start to end
     */
    std::vector<XYZCoord> generatePoints(const RRTPoint &start, const RRTPoint &end,
                                       const DubinsPath &path, bool has_straight) const;

    /**
     *  First, the straight distance (it turns out) is equal to the
     *  distance between the two centers
     *  Next it calculates the turning angle (positive [CCW] turn from start to
     *  end vector)
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
    RRTOption lsl(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                  const XYZCoord &center_2) const;

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
    RRTOption rsr(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                  const XYZCoord &center_2) const;

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
    RRTOption lsr(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                  const XYZCoord &center_2) const;

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
    RRTOption rsl(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                  const XYZCoord &center_2) const;

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
    RRTOption lrl(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                  const XYZCoord &center_2) const;

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
    RRTOption rlr(const RRTPoint &start, const RRTPoint &end, const XYZCoord &center_0,
                  const XYZCoord &center_2) const;

    /**
     * Compute all the possible Dubin's path and returns a list
     *  containing RRTOption(s) with path data.
     *
     *  @param start    ==> vector at start position
     *  @param end      ==> vector at end position
     *  @param sort     ==> whether the method sorts the resulting vector
     *                      DEFALT-->FALSE (searching is faster)
     *  @return         ==> list containing all the RRTOptions from the path
     *                      generation
     */
    std::vector<RRTOption> allOptions(const RRTPoint &start, const RRTPoint &end,
                                      bool sort = false) const;

    /**
     * Compute all the possible Dubin's path(s) and
     * Returns sequence of points representing the shortest option.
     *
     *  @param start    ==> vector at start position
     *  @param end      ==> vector at end position
     *  @return         ==> the points for the most optimal path from @param start to @param end
     */
    std::vector<XYZCoord> dubinsPath(const RRTPoint &start, const RRTPoint &end) const;

    /**
     * Returns the optimal RRTOption from the list of options
     *
     * @param start     ==> vector at start position
     * @param end       ==> vector at end position
     * @return          ==> the optimal RRTOption from the list of options
     */
    RRTOption bestOption(const RRTPoint &start, const RRTPoint &end) const;

 private:
    const double _radius;
    const double _point_separation;
};

#endif  // INCLUDE_PATHING_DUBINS_HPP_
