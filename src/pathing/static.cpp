#include "pathing/static.hpp"

#include <httplib.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "core/mission_state.hpp"
#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "pathing/plotting.hpp"
#include "pathing/rrt.hpp"
#include "pathing/tree.hpp"
#include "utilities/common.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/obc_config.hpp"
#include "utilities/rng.hpp"

ForwardCoveragePathing::ForwardCoveragePathing(const RRTPoint& start, double scan_radius,
                                               const OBCConfig& config)
    : scan_radius(scan_radius), start(start), config(config.pathing.coverage) {}

std::vector<XYZCoord> ForwardCoveragePathing::run() const {
    // return coverageDefault();
    return config.forward.optimize ? coverageOptimal() : coverageDefault();
}

std::vector<XYZCoord> ForwardCoveragePathing::coverageDefault() const {
    RRT rrt = pathScanLines(config.forward.one_way, config.forward.vertical);
    rrt.generateFlightPoints();

    return rrt.getPointsToGoal();
}

std::vector<XYZCoord> ForwardCoveragePathing::coverageOptimal() const {
    /*
     * The order of paths
     * [0] - alt, vertical
     * [1] - alt, horizontal
     * [2] - one_way, vertical
     * [3] - one_way, horizontal
     */

    const std::vector<std::pair<bool, bool>> layouts = {
        {false, true}, {false, false}, {true, true}, {true, false}};

    /*
     * Which layout is cheapest cannot be told from the scan lines alone -- what a
     * layout costs is the flying it takes to get from one line to the next and
     * around whatever is in the way, which is not known until it has been pathed.
     * So all four are pathed, and only the one that wins is ever flown.
     */
    std::optional<RRT> best;

    for (const std::pair<bool, bool>& layout : layouts) {
        RRT rrt = pathScanLines(layout.first, layout.second);

        if (!best.has_value() || rrt.pathLength() < best->pathLength()) {
            best.emplace(std::move(rrt));
        }
    }

    if (!best.has_value()) {
        return {};
    }

    best->generateFlightPoints();
    return best->getPointsToGoal();
}

std::vector<RRTPoint> ForwardCoveragePathing::scanLines(bool one_way, bool vertical) const {
    std::vector<RRTPoint> waypoints =
        Environment::getAirdropWaypoints(scan_radius, one_way, vertical);

    // the whole sweep is flown at one altitude, so only the way in is a climb
    for (RRTPoint& waypoint : waypoints) {
        waypoint.coord.z = config.altitude_m;
    }

    // the plane flies from where it is now, so that is the first of the waypoints
    waypoints.insert(waypoints.begin(), start);

    return waypoints;
}

RRT ForwardCoveragePathing::pathScanLines(bool one_way, bool vertical) const {
    const std::vector<RRTPoint> waypoints = scanLines(one_way, vertical);

    std::vector<XYZCoord> goals;
    goals.reserve(waypoints.size());

    /*
     * A scan line only covers the ground it is meant to if it is flown along its
     * own direction, so each waypoint is left exactly one way to be reached and
     * RRT is only free to choose the flying between them.
     */
    std::vector<std::vector<double>> goal_angles;
    goal_angles.reserve(waypoints.size());

    for (const RRTPoint& waypoint : waypoints) {
        goals.push_back(waypoint.coord);
        goal_angles.push_back({waypoint.psi});
    }

    RRT rrt(std::move(goals), start.psi, std::move(goal_angles));
    rrt.generateDubinsOptions();

    return rrt;
}

HoverCoveragePathing::HoverCoveragePathing(std::shared_ptr<MissionState> state)
    : config{state->config.pathing.coverage},
      drop_zone{state->mission_params.getAirdropBoundary()},
      state{state} {}

// Function to calculate the center of the polygon
XYZCoord calculateCenter(const std::vector<XYZCoord>& polygon) {
    XYZCoord center(0.0, 0.0, 0.0);
    for (const auto& point : polygon) {
        center.x += point.x;
        center.y += point.y;
    }
    center.x /= polygon.size();
    center.y /= polygon.size();
    return center;
}

// Function to scale the polygon
void scalePolygon(std::vector<XYZCoord>& polygon, double scaleFactor) {
    // Step 1: Calculate the center of the polygon
    XYZCoord center = calculateCenter(polygon);

    // Step 2: Translate the polygon to the origin
    for (auto& point : polygon) {
        point.x -= center.x;
        point.y -= center.y;
    }

    // Step 3: Scale the polygon
    for (auto& point : polygon) {
        point.x *= scaleFactor;
        point.y *= scaleFactor;
    }

    // Step 4: Translate the polygon back to its original center
    for (auto& point : polygon) {
        point.x += center.x;
        point.y += center.y;
    }
}

std::vector<XYZCoord> HoverCoveragePathing::run() {
    if (this->drop_zone.size() != 4) {
        // right now just hardcoded to rectangles. think its ok to panic here because
        // we would want to stop early if we messed this up and this will happen
        // before takeoff
        LOG_F(FATAL, "Hover airdrop pathing currently only supports 4 coordinates, not %lu",
              this->drop_zone.size());
    }

    // Input Coordinates MUST BE in this order
    XYZCoord bottom_left = this->drop_zone.at(0);
    XYZCoord bottom_right = this->drop_zone.at(1);
    XYZCoord top_right = this->drop_zone.at(2);
    XYZCoord top_left = this->drop_zone.at(3);

    std::vector<XYZCoord> hover_points;

    double vision = this->config.camera_vision_m;
    double altitude = this->config.altitude_m;

    double start_y = std::max(top_left.y, top_right.y) - (vision / 2.0);
    double stop_y = std::min(bottom_left.y, bottom_right.y) - (vision / 2.0);
    double start_x = std::min(top_left.x, bottom_left.x) + (vision / 2.0);
    double stop_x = std::max(top_right.x, bottom_right.x) + (vision / 2.0);

    Polygon scaled_drop_zone = this->drop_zone;
    scalePolygon(scaled_drop_zone, 1.20);

    bool right = true;  // start going from right to left
    for (double y = start_y; y > stop_y; y -= vision) {
        std::vector<XYZCoord> row;  // row of points either from left to right or right to left
        for (double x = start_x; x < stop_x; x += vision) {
            XYZCoord pt(x, y, altitude);
            if (Environment::isPointInPolygon(scaled_drop_zone, pt)) {
                row.push_back(pt);
            }
        }
        if (!right) {
            std::reverse(row.begin(), row.end());
        }
        right = !right;
        hover_points.insert(std::end(hover_points), std::begin(row), std::end(row));
    }

    return hover_points;
}

AirdropApproachPathing::AirdropApproachPathing(const RRTPoint& start, const XYZCoord& goal,
                                               XYZCoord wind, const OBCConfig& config)
    : goal(goal), start(start), config(config), wind(wind) {}

std::vector<XYZCoord> AirdropApproachPathing::run() const {
    RRTPoint drop_vector = getDropLocation();

    // the drop is only a drop if it is flown at the heading that lines the plane
    // up with the target, so that is the one way the goal may be reached
    const std::vector<double> approach_angles = {drop_vector.psi};

    RRT rrt({start.coord, drop_vector.coord}, start.psi, approach_angles);
    rrt.run();

    return rrt.getPointsToGoal();
}

RRTPoint AirdropApproachPathing::getDropLocation() const {
    double drop_angle = config.pathing.approach.drop_angle_rad;
    double drop_distance = 0.0f;
    if (config.pathing.approach.drop_method == AirdropDropMethod::Enum::GUIDED) {
        drop_distance = config.pathing.approach.guided_drop_distance_m;
    } else {
        drop_distance = config.pathing.approach.unguided_drop_distance_m;
    }

    XYZCoord drop_offset(drop_distance * std::cos(drop_angle), drop_distance * std::sin(drop_angle),
                         0);

    double wind_strength_coef = wind.norm() * WIND_CONST_PER_ALTITUDE;
    double wind_angle = std::atan2(wind.y, wind.x);
    XYZCoord wind_offset(wind_strength_coef * std::cos(wind_angle),
                         wind_strength_coef * std::sin(wind_angle), 0);
    XYZCoord drop_location(goal.x + drop_offset.x + wind_offset.x,
                           goal.y + drop_offset.y + wind_offset.y,
                           config.pathing.approach.drop_altitude_m);

    // gets the angle between the drop_location and the goal
    double angle = std::atan2(goal.y - drop_location.y, goal.x - drop_location.x);
    return RRTPoint(drop_location, angle);
}

RRTPoint getCurrentLoc(std::shared_ptr<MissionState> state) {
    std::shared_ptr<MavlinkClient> mav = state->getMav();
    std::pair<double, double> start_lat_long = mav->latlng_deg();

    GPSCoord start_gps =
        makeGPSCoord(start_lat_long.first, start_lat_long.second, mav->altitude_agl_m());

    double angle_correction = (90 - mav->heading_deg()) * M_PI / 180.0;
    double start_angle = (angle_correction < 0) ? (angle_correction + 2 * M_PI) : angle_correction;
    XYZCoord start_xyz = state->getCartesianConverter().value().toXYZ(start_gps);
    return RRTPoint(start_xyz, start_angle);
}

double calculateFinalAngle(
    const MissionPath& path,
    const std::optional<CartesianConverter<GPSProtoVec>>& cartesianConverter) {
    const auto& coords = path.get();
    if (coords.size() < 2) {
        return 0.0;  // should probably either have angle between now and point
                     // or use assert for force size >=2
    }

    XYZCoord pt1 = cartesianConverter->toXYZ(coords[coords.size() - 2]);
    XYZCoord pt2 = cartesianConverter->toXYZ(coords[coords.size() - 1]);

    return std::atan2(pt2.y - pt1.y, pt2.x - pt1.x);
}

std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state) {
    // first waypoint is start

    // the other waypoitns is the goals
    if (state->mission_params.getWaypoints().size() < 1) {
        loguru::set_thread_name("Static Pathing");
        LOG_F(ERROR, "Not enough waypoints to generate a path, requires >=1, num waypoints: %s",
              std::to_string(state->mission_params.getWaypoints().size()).c_str());
        return {};
    }

    std::vector<XYZCoord> goals = state->mission_params.getWaypoints();

    RRTPoint start = getCurrentLoc(state);
    start.coord.z = state->config.takeoff.altitude_m;

    // the plane flies from where it is now, so that is the first of the waypoints
    goals.insert(goals.begin(), start.coord);

    RRT rrt(goals, start.psi);

    rrt.run();

    std::vector<XYZCoord> path = rrt.getPointsToGoal();

    std::vector<GPSCoord> output_coords;
    for (const XYZCoord& waypoint : path) {
        output_coords.push_back(state->getCartesianConverter()->toLatLng(waypoint));
    }

    return output_coords;
}

std::vector<GPSCoord> generateNextWaypointPath(std::shared_ptr<MissionState> state,
                                               double start_angle) {
    if (state->mission_params.getWaypoints().size() < 1) {
        loguru::set_thread_name("Static Pathing");
        LOG_F(ERROR, "Not enough waypoints to generate a path, requires >=1, num waypoints: %s",
              std::to_string(state->mission_params.getWaypoints().size()).c_str());
        return {};
    }

    std::vector<XYZCoord> goals = state->mission_params.getWaypoints();

    RRTPoint start(goals.back(), start_angle);

    // add buffer to the start point so that we dont loopty loop
    double buffer_m = state->config.pathing.upload_distance_buffer_m;
    if (buffer_m > 0.0) {
        start.coord.x += buffer_m * std::cos(start_angle);
        start.coord.y += buffer_m * std::sin(start_angle);
    }

    // the plane flies from where it is now, so that is the first of the waypoints
    goals.insert(goals.begin(), start.coord);

    RRT rrt(goals, start.psi);

    rrt.run();

    std::vector<XYZCoord> path = rrt.getPointsToGoal();

    std::vector<GPSCoord> output_coords;
    for (const XYZCoord& waypoint : path) {
        output_coords.push_back(state->getCartesianConverter()->toLatLng(waypoint));
    }

    return output_coords;
}

std::vector<GPSCoord> generateSearchPath(std::shared_ptr<MissionState> state, double start_angle) {
    std::vector<GPSCoord> gps_coords;
    if (state->config.pathing.coverage.method == AirdropCoverageMethod::Enum::FORWARD) {
        if (state->mission_params.getWaypoints().size() < 1) {
            loguru::set_thread_name("Static Pathing");
            LOG_F(ERROR,
                "Waypoint path is empty. Failed to generate search path");
            return {};
        }
        RRTPoint start(state->mission_params.getWaypoints().back(), start_angle);

        double scan_radius = state->config.pathing.coverage.camera_vision_m;

        ForwardCoveragePathing pathing(start, scan_radius, state->config);

        for (const auto& coord : pathing.run()) {
            gps_coords.push_back(state->getCartesianConverter()->toLatLng(coord));
        }

        return gps_coords;
    } else {  // hover
        HoverCoveragePathing pathing(state);

        for (const auto& coord : pathing.run()) {
            gps_coords.push_back(state->getCartesianConverter()->toLatLng(coord));
        }
        return gps_coords;
    }
}

std::vector<GPSCoord> generateAirdropApproach(std::shared_ptr<MissionState> state,
                                              const GPSCoord& goal) {
    std::shared_ptr<MavlinkClient> mav = state->getMav();
    /*
        Note: this function was neutered right before we attempted to fly at the 2024 competition
        because we suddenly began running into an infinite loop during the execution of this
        function. Instead of spending an undeterministic amount of time to fix this problem,
        we ended up relying solely on Arduplane to navigate to the specified drop point
        instead of trying to formulate our own path.
    */

    RRTPoint start_rrt = getCurrentLoc(state);
    // pathing
    XYZCoord goal_xyz = state->getCartesianConverter().value().toXYZ(goal);
    AirdropApproachPathing airdrop_planner(start_rrt, goal_xyz, mav->wind(), state->config);
    std::vector<XYZCoord> xyz_path = airdrop_planner.run();

    // try to fly to the third waypoint in the path
    // prevents the drone from passing the initial waypoint
    // [TODO]-done out of laziness, forgot if the path includes starting location
    xyz_path.erase(xyz_path.begin());
    xyz_path.erase(xyz_path.begin());

    std::vector<GPSCoord> gps_path;
    // XYZCoord pt = state->getCartesianConverter().value().toXYZ(goal);

    for (const XYZCoord& wpt : xyz_path) {
        gps_path.push_back(state->getCartesianConverter().value().toLatLng(wpt));
    }

    // there is I think an off by one error on the timing of the airdrop if there
    // is only one coordinate (mav command) in this mission
    // (in essence it will instantly think the mission over or almost over instead of waiting
    // for it to reach the singular and final waypoint).
    // So in the hours before competition 2024 instead of fixing this I came across
    // this wonderful solution which was revealed to me in a dream.
    // gps_path.push_back(goal);
    // gps_path.push_back(goal);
    // gps_path.push_back(goal);
    // gps_path.push_back(goal);
    // gps_path.push_back(goal);

    return gps_path;
}
