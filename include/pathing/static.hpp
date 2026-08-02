#ifndef INCLUDE_PATHING_STATIC_HPP_
#define INCLUDE_PATHING_STATIC_HPP_

#include <httplib.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "core/mission_state.hpp"
#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "pathing/mission_path.hpp"
#include "pathing/plotting.hpp"
#include "pathing/rrt.hpp"
#include "pathing/tree.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"


/**
 * Class that performs Coverage-Path_Planning (CPP) over a given polygon
 *
 * Basically draws vertical lines, and then connects them with RRT, which keeps
 * the legs between the lines inside the airspace
 *
 * Limitations
 * - Cannot path through non-convex shapes
 *
 * Notes:
 * - this implementation is for fixed wing planes, which is not currently being used. However,
 *   it is kept here because it is very possible we will eventually switch back to it.
 */
class ForwardCoveragePathing {
 public:
    ForwardCoveragePathing(const RRTPoint &start, double scan_radius, const OBCConfig &config);

    /**
     * Generates a path of parallel lines to cover a given area
     *
     * TODO - optimize dubins to not have to go to each line, rather search every other line then
     * loop back
     *
     * @return  ==> list of 2-vectors describing the path through the aridrop_zone
     */
    std::vector<XYZCoord> run() const;

    /**
     *   The algorithm run if not optimizing the path legnth
     */
    std::vector<XYZCoord> coverageDefault() const;

    /**
     *   The algorithm run if optimizing path length
     */
    std::vector<XYZCoord> coverageOptimal() const;

    /**
     * The waypoints that sweep the zone with one layout of scan lines, starting
     * from where the plane is now
     *
     * These are the mission, not the path -- each one carries the heading its
     * line has to be flown at, and RRT is what works out how to get from one to
     * the next.
     *
     * @param one_way   ==> whether every line is flown in the same direction,
     *                      rather than alternating
     * @param vertical  ==> whether the lines run vertically
     */
    std::vector<RRTPoint> scanLines(bool one_way, bool vertical) const;

    /**
     * Searches out the dubins paths that fly one layout of scan lines
     *
     * The points along them are not generated, so the caller may weigh the
     * mission against another one and throw it away cheaply.
     *
     * @param one_way   ==> whether every line is flown in the same direction,
     *                      rather than alternating
     * @param vertical  ==> whether the lines run vertically
     */
    RRT pathScanLines(bool one_way, bool vertical) const;

 private:
    const double scan_radius;  // how far each side of the plane we intend to look (half dist
                               // between search lines)
    const RRTPoint start;      // start location (doesn't have to be near polygon)
    const AirdropCoverageConfig config;
};

/**
 * Class that performs coverage pathing over a given search area, given that the plane has
 * hovering capabilities and that we want to be taking pictures while hovering over the zone.
 *
 * This outputs a series of XYZ Coordinates which represent a points at which the plane
 * should hover and take a picture.
 *
 * Assumptions:
 * - The drop zone has 4 points which form a rectangle larger than the vision of the camera
 */
class HoverCoveragePathing {
 public:
    explicit HoverCoveragePathing(std::shared_ptr<MissionState> state);

    std::vector<XYZCoord> run();

 private:
    std::shared_ptr<MissionState> state;
    AirdropCoverageConfig config;
    Polygon drop_zone;
};

class AirdropApproachPathing {
 public:
    AirdropApproachPathing(const RRTPoint &start, const XYZCoord &goal, XYZCoord wind,
                           const OBCConfig &config);
    /**
     * Generates a path to the drop location
     *
     * @return  ==> list of 2-vectors describing the path to the drop location
     */
    std::vector<XYZCoord> run() const;

    /**
     * Generates the vector to the drop location
     */
    RRTPoint getDropLocation() const;

 private:
    const XYZCoord goal;
    const RRTPoint start;
    const OBCConfig config;

    XYZCoord wind;
};

/**
 * Helper function to get the current location of the plane for pathing
 * 
 * @param state ==> the mission state to get the mavlink client from
 * @return      ==> RRTPoint representing the current location
 */
RRTPoint getCurrentLoc(std::shared_ptr<MissionState> state);

/**
 * Calculates the approximate angle of exit out of a waypoint path
 * 
 * @param path  ==> the waypoint path
 * @param state ==> mission state
 * @return          angle of exit (+x-axis is 0 deg, ccw is positive)
 */
double calculateFinalAngle(
    const MissionPath& path,
    const std::optional<CartesianConverter<GPSProtoVec>>& cartesianConverter);

std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state);

std::vector<GPSCoord>
generateNextWaypointPath(std::shared_ptr<MissionState> state, double start_angle);

std::vector<GPSCoord>
generateSearchPath(std::shared_ptr<MissionState> state, double start_angle);

std::vector<GPSCoord>
generateAirdropApproach(std::shared_ptr<MissionState> state, const GPSCoord &goal);

#endif  // INCLUDE_PATHING_STATIC_HPP_
