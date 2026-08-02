#ifndef INCLUDE_PATHING_PATH_GENERATOR_HPP_
#define INCLUDE_PATHING_PATH_GENERATOR_HPP_

#include <vector>

#include "pathing/rrt.hpp"
#include "utilities/datatypes.hpp"

/**
 * Flies a mission through a list of waypoints.
 */
class PathGenerator {
 public:
    RRT rrt;
    std::vector<XYZCoord> flight_path;
    const std::vector<XYZCoord> goals;
    const std::vector<std::vector<double>> goal_angles;  // Final Appoach Angles
    std::vector<Leg> legs;                               // Legs of Mission

    /**
     * @param[in] goals         ==> the waypoints to fly through, in order, the
     *                              first of which is where the plane already is
     * @param[in] start_angle   ==> the heading the plane is flying at right now
     * @param[in] goal_angles   ==> the angles each goal may be approached at
     */
    PathGenerator(std::vector<XYZCoord> goals, double start_angle,
                  std::vector<std::vector<double>> goal_angles);

    /**
     * @param[in] goals         ==> the waypoints to fly through, in order, the
     *                              first of which is where the plane already is
     * @param[in] start_angle   ==> the heading the plane is flying at right now
     * @param[in] angles        ==> the angles every goal may be approached at
     */
    PathGenerator(std::vector<XYZCoord> goals, double start_angle,
                  std::vector<double> angles = {});

    /**
     * Searches out the mission and then flies it
     */
    void run();

    /**
     * Generates the Optimal Dubin's Options to fly the waypoints
     */
    void generateDubinsOptions();

    /**
     * Generates the points to fly
     */
    void generateFlightPoints();

    /**
     * @return  ==> the sum of the length of every leg
     */
    double pathLength() const;

    /**
     * returns a continuous path of points to the goal
     *
     * @return  ==> list of 2-vectors to the goal region
     */
    std::vector<XYZCoord> getPointsToGoal() const;

    /**
     * The points flown along one leg, including altitude
     *
     * @param[in] leg   ==> the leg to fly
     * @return  ==> the points along the leg, at altitude
     */
    std::vector<XYZCoord> buildFlightPath(const Leg &leg) const;
};

#endif  // INCLUDE_PATHING_PATH_GENERATOR_HPP_
