#include "pathing/static.hpp"

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
#include "pathing/plotting.hpp"
#include "pathing/tree.hpp"
#include "utilities/common.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/obc_config.hpp"
#include "utilities/rng.hpp"

RRT::RRT(RRTPoint start, std::vector<XYZCoord> goals, double search_radius, Polygon bounds,
         const OBCConfig &config, std::vector<Polygon> obstacles, std::vector<double> angles)
    : iterations_per_waypoint(config.pathing.rrt.iterations_per_waypoint),
      search_radius(search_radius),
      rewire_radius(config.pathing.rrt.rewire_radius),
      tree(start, Environment(bounds, {}, {}, goals, obstacles),
           Dubins(config.pathing.dubins.turning_radius, config.pathing.dubins.point_separation)),
      config(config.pathing.rrt) {
    if (angles.size() != 0) {
        this->angles = angles;
    }
}

RRT::RRT(RRTPoint start, std::vector<XYZCoord> goals, double search_radius, Environment airspace,
         const OBCConfig &config, std::vector<double> angles)
    : iterations_per_waypoint(config.pathing.rrt.iterations_per_waypoint),
      search_radius(search_radius),
      rewire_radius(config.pathing.rrt.rewire_radius),
      tree(start, airspace,
           Dubins(config.pathing.dubins.turning_radius, config.pathing.dubins.point_separation)),
      config(config.pathing.rrt) {
    if (angles.size() != 0) {
        this->angles = angles;
    }
}

void RRT::run() {
    /*
     * RRT algorithm
     * - Treats each waypoint as a goal, DOES NOT reuse trees between waypoints,
     *    basically calls RRT for each waypoint
     * - For Each Waypoint
     *  - Tries to connect directly to the goal
     *  - If it can't, it runs the RRT algorithm
     *      - Attempts to converge based on epoch intervals
     *      - If it can't, it connects to the goal with whatever it has
     */
    const int total_goals = tree.getAirspace().getNumGoals();

    for (int current_goal_index = 0; current_goal_index < total_goals; current_goal_index++) {
        // tries to connect directly to the goal
        if (connectToGoal(current_goal_index)) {
            continue;
        }

        // run the RRT algorithm if it can not connect
        RRTIteration(iterations_per_waypoint, current_goal_index);
    }
}

std::vector<XYZCoord> RRT::getPointsToGoal() const {
    // return tree.getPathToGoal();
    return flight_path;
}

bool RRT::RRTIteration(int tries, int current_goal_index) {
    const int epoch_interval = tries / NUM_EPOCHS;
    int current_epoch = epoch_interval;

    RRTNode *goal_node = nullptr;

    for (int i = 0; i < tries; i++) {
        if (i == current_epoch) {
            // generates a new node (not connect), and adds and breaks if it is
            // within X% of the last generation
            if (epochEvaluation(goal_node, current_goal_index)) {
                return true;
            }

            current_epoch += epoch_interval;
        }
        // generate a sample point
        const RRTPoint sample = generateSamplePoint();

        // returns all dubins options from the tree to the sample
        const std::vector<std::pair<RRTNode *, RRTOption>> &options =
            tree.pathingOptions(sample, config.point_fetch_method);

        // returns true if the node is successfully added to the tree
        RRTNode *new_node = parseOptions(options, sample);

        if (new_node != nullptr && config.optimize) {
            optimizeTree(new_node);
        }
    }

    // frees memory
    delete (goal_node);
    if (!connectToGoal(current_goal_index)) {
        loguru::set_thread_name("Static Pathing");
        LOG_F(WARNING, "Failed to connect to goal on iteration: [%s]. Trying again...",
              std::to_string(current_goal_index).c_str());

        if (!config.allowed_to_skip_waypoints &&
            !connectToGoal(current_goal_index, std::numeric_limits<int>::max())) {
            // will always return true (unless it turns into a pseudo-infinite loop)
            return RRTIteration(tries, current_goal_index);
        } else {
            return false;
        }
    }

    return true;
}

bool RRT::epochEvaluation(RRTNode *goal_node, int current_goal_index) {
    // If a single epoch has not been passed, mark this goal as the first
    // benchmark.
    if (goal_node == nullptr) {
        goal_node = sampleToGoal(current_goal_index, TOTAL_OPTIONS_FOR_GOAL_CONNECTION);
        return false;
    }

    RRTNode *new_node = sampleToGoal(current_goal_index, TOTAL_OPTIONS_FOR_GOAL_CONNECTION);

    if (new_node == nullptr) {
        return false;
    }

    /* If the new node is within ~X% of the goal, then we are done.
     * It should be impossible for new_node to be more inefficient than
     * goal_node as it uses a superset of the tree goal_node used
     */
    if (new_node->getCost() < EPOCH_TEST_MARGIN * goal_node->getCost()) {
        delete (goal_node);
        goal_node = new_node;
        return false;
    }

    addNodeToTree(new_node, current_goal_index);
    return true;
}

RRTPoint RRT::generateSamplePoint() const { return tree.getRandomPoint(search_radius); }

std::vector<std::pair<RRTPoint, std::pair<RRTNode *, RRTOption>>> RRT::getOptionsToGoal(
    int current_goal_index, int total_options) const {
    // attempts to connect to the goal, should always connect
    std::vector<RRTPoint> goal_points;

    // Generates goal specific points based on current Waypoints and list og
    // Angles
    for (const double angle : angles) {
        const XYZCoord &goal = tree.getAirspace().getGoal(current_goal_index);
        goal_points.push_back(RRTPoint(goal, angle));
    }

    // RRTPoint is the goal that is to be connected
    // RRTNode is the node in the tree that is the anchor
    // RRTOPtion Node-->Point
    std::vector<std::pair<RRTPoint, std::pair<RRTNode *, RRTOption>>> all_options;

    // limit amount of options to sort, defined in constants.hpp
    const int NUMBER_OPTIONS_EACH = total_options / angles.size();

    // gets all options for each of the goals, and puts them into a unified list
    // TODO ? maybe better for a max heap?
    for (const RRTPoint &goal : goal_points) {
        const std::vector<std::pair<RRTNode *, RRTOption>> &options =
            // For now, we use optimal pathing
            tree.pathingOptions(goal, PointFetchMethod::Enum::NONE, NUMBER_OPTIONS_EACH);

        for (const auto &[node, option] : options) {
            all_options.push_back({goal, {node, option}});
        }
    }

    std::sort(all_options.begin(), all_options.end(), [](const auto &a, const auto &b) {
        auto &[a_goal, a_paths] = a;
        auto &[a_node, a_option] = a_paths;
        auto &[b_goal, b_paths] = b;
        auto &[b_node, b_option] = b_paths;
        return a_option.length + a_node->getCost() < b_option.length + b_node->getCost();
    });

    return all_options;
}

RRTNode *RRT::sampleToGoal(int current_goal_index, int total_options) const {
    // gets all options for each of the goals
    const auto &all_options = getOptionsToGoal(current_goal_index, total_options);

    // <RRTPoint GOAL, {RRTNode* ANCHOR, RRTOption} >
    for (const auto &[goal, pair] : all_options) {
        auto &[anchor_node, option] = pair;

        RRTNode *new_node = tree.generateNode(anchor_node, goal, option);

        if (new_node != nullptr) {
            return new_node;
        }
    }

    return nullptr;
}

bool RRT::connectToGoal(int current_goal_index, int total_options) {
    RRTNode *goal_node = sampleToGoal(current_goal_index, total_options);

    if (goal_node == nullptr) {
        return false;
    }

    addNodeToTree(goal_node, current_goal_index);
    return true;
}

void RRT::addNodeToTree(RRTNode *goal_node, int current_goal_index) {
    // add the node to the tree
    tree.addNode(goal_node->getParent(), goal_node);

    // inserts the altitude into the path
    std::vector<XYZCoord> local_path = tree.getPathSegment(goal_node);

    double start_height;
    if (current_goal_index == 0) {
        start_height = tree.getStart().coord.z;
    } else {
        start_height = tree.getAirspace().getGoal(current_goal_index - 1).z;
    }

    double height_difference = tree.getAirspace().getGoal(current_goal_index).z - start_height;
    double height_increment = height_difference / local_path.size();

    for (XYZCoord &point : local_path) {
        point.z = start_height;
        start_height += height_increment;
    }

    // adds local path to the flight path, and updates the tree
    flight_path.insert(flight_path.end(), local_path.begin(), local_path.end());
    tree.setCurrentHead(goal_node);
}

RRTNode *RRT::parseOptions(const std::vector<std::pair<RRTNode *, RRTOption>> &options,
                           const RRTPoint &sample) {
    for (auto &[node, option] : options) {
        /*
         *  stop if
         *  1. the node is null
         *  2. the node is the same as the sample
         *
         *  The idea is that any further options will have the same if not more
         * issues
         *
         * This shouldn't ever happen?
         */
        // if (node == nullptr || node->getPoint() == sample) {
        //     return nullptr;

        // else, attempt to add the node to the tree
        RRTNode *sucessful_addition = tree.addSample(node, sample, option);

        if (sucessful_addition != nullptr) {
            return sucessful_addition;
        }
    }

    return nullptr;
}

void RRT::optimizeTree(RRTNode *sample) { tree.RRTStar(sample, rewire_radius); }

ForwardCoveragePathing::ForwardCoveragePathing(const RRTPoint &start, double scan_radius,
                                               Polygon bounds, Polygon airdrop_zone,
                                               const OBCConfig &config,
                                               std::vector<Polygon> obstacles)
    : start(start),
      scan_radius(scan_radius),
      airspace(Environment(bounds, airdrop_zone, {}, {}, obstacles)),
      dubins(Dubins(config.pathing.dubins.turning_radius, config.pathing.dubins.point_separation)),
      config(config.pathing.coverage) {}

std::vector<XYZCoord> ForwardCoveragePathing::run() const {
    return config.forward.optimize ? coverageOptimal() : coverageDefault();
}

std::vector<XYZCoord> ForwardCoveragePathing::coverageDefault() const {
    // generates the endpoints for the lines (including headings)
    std::vector<RRTPoint> waypoints =
        airspace.getAirdropWaypoints(scan_radius, config.forward.one_way, config.forward.vertical);
    waypoints.emplace(waypoints.begin(), start);

    // generates the path connecting the q
    std::vector<RRTOption> dubins_options;
    for (int i = 0; i < waypoints.size() - 1; i++) {
        dubins_options.push_back(dubins.bestOption(waypoints[i], waypoints[i + 1]));
    }

    return generatePath(dubins_options, waypoints);
}

std::vector<XYZCoord> ForwardCoveragePathing::coverageOptimal() const {
    /*
     * The order of paths
     * [0] - alt, vertical
     * [1] - alt, horizontal
     * [2] - one_way, vertical
     * [3] - one_way, horizontal
     */

    std::vector<std::pair<bool, bool>> configs = {
        {false, true}, {false, false}, {true, true}, {true, false}};

    std::vector<std::vector<RRTOption>> dubins_paths;
    std::vector<int> lengths = {0, 0, 0, 0};

    // generates the endpoints for the lines (including headings)
    for (int i = 0; i < configs.size(); i++) {
        const auto &config = configs[i];

        std::vector<RRTPoint> waypoints =
            airspace.getAirdropWaypoints(scan_radius, config.first, config.second);

        // generates the path connecting the waypoints to each other
        std::vector<RRTOption> current_dubins_path;

        for (int i = 0; i < waypoints.size() - 1; i++) {
            RRTOption dubins_path = dubins.bestOption(waypoints[i], waypoints[i + 1]);
            lengths[i] += dubins_path.length;
            current_dubins_path.push_back(dubins_path);
        }

        dubins_paths.push_back(current_dubins_path);
    }

    // finds the shortest path
    int best_path_idx = 0;
    double shortest_length = lengths[0];
    for (int i = 1; i < lengths.size(); i++) {
        if (lengths[i] < shortest_length) {
            shortest_length = lengths[i];
            best_path_idx = i;
        }
    }

    // gets the path
    std::vector<RRTPoint> waypoints = airspace.getAirdropWaypoints(
        scan_radius, configs[best_path_idx].first, configs[best_path_idx].second);

    waypoints.emplace(waypoints.begin(), start);

    return generatePath(dubins_paths[best_path_idx], waypoints);
}

std::vector<XYZCoord> ForwardCoveragePathing::generatePath(
    const std::vector<RRTOption> &dubins_options, const std::vector<RRTPoint> &waypoints) const {
    std::vector<XYZCoord> path;

    // height adjustement
    double height = waypoints[0].coord.z;
    double height_difference = config.altitude_m - waypoints[0].coord.z;

    std::vector<XYZCoord> path_coordinates = dubins.generatePoints(
        waypoints[0], waypoints[1], dubins_options[0].dubins_path, dubins_options[0].has_straight);

    double height_increment = height_difference / path_coordinates.size();

    for (XYZCoord &coord : path_coordinates) {
        coord.z = height;
        height += height_increment;
    }

    path.insert(path.end(), path_coordinates.begin() + 1, path_coordinates.end());

    // main loop
    for (int i = 1; i < dubins_options.size(); i++) {
        path_coordinates =
            dubins.generatePoints(waypoints[i], waypoints[i + 1], dubins_options[i].dubins_path,
                                  dubins_options[i].has_straight);

        for (XYZCoord &coord : path_coordinates) {
            coord.z = config.altitude_m;
        }

        path.insert(path.end(), path_coordinates.begin() + 1, path_coordinates.end());
    }

    return path;
}

HoverCoveragePathing::HoverCoveragePathing(std::shared_ptr<MissionState> state)
    : config{state->config.pathing.coverage},
      drop_zone{state->mission_params.getAirdropBoundary()},
      state{state} {}

// Function to calculate the center of the polygon
XYZCoord calculateCenter(const std::vector<XYZCoord> &polygon) {
    XYZCoord center(0.0, 0.0, 0.0);
    for (const auto &point : polygon) {
        center.x += point.x;
        center.y += point.y;
    }
    center.x /= polygon.size();
    center.y /= polygon.size();
    return center;
}

// Function to scale the polygon
void scalePolygon(std::vector<XYZCoord> &polygon, double scaleFactor) {
    // Step 1: Calculate the center of the polygon
    XYZCoord center = calculateCenter(polygon);

    // Step 2: Translate the polygon to the origin
    for (auto &point : polygon) {
        point.x -= center.x;
        point.y -= center.y;
    }

    // Step 3: Scale the polygon
    for (auto &point : polygon) {
        point.x *= scaleFactor;
        point.y *= scaleFactor;
    }

    // Step 4: Translate the polygon back to its original center
    for (auto &point : polygon) {
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

AirdropApproachPathing::AirdropApproachPathing(const RRTPoint &start, const XYZCoord &goal,
                                               XYZCoord wind, Polygon bounds,
                                               const OBCConfig &config,
                                               std::vector<Polygon> obstacles)
    : start(start),
      goal(goal),
      wind(wind),
      airspace(Environment(bounds, {}, {}, {goal}, obstacles)),
      dubins(Dubins(config.pathing.dubins.turning_radius, config.pathing.dubins.point_separation)),
      config(config) {}

std::vector<XYZCoord> AirdropApproachPathing::run() const {
    RRTPoint drop_vector = getDropLocation();
    RRT rrt(start, {drop_vector.coord}, SEARCH_RADIUS, airspace, config, {drop_vector.psi});
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

std::vector<std::vector<XYZCoord>> generateGoalListDeviations(const std::vector<XYZCoord> &goals,
                                                              XYZCoord deviation_point) {
    std::vector<std::vector<XYZCoord>> goal_list_deviations;
    for (int i = 0; i < goals.size() + 1; i++) {
        std::vector<XYZCoord> goal_list_deviation = goals;
        goal_list_deviation.insert(goal_list_deviation.begin() + i, deviation_point);
        goal_list_deviations.push_back(goal_list_deviation);
    }

    return goal_list_deviations;
}

std::vector<std::vector<XYZCoord>> generateRankedNewGoalsList(const std::vector<XYZCoord> &goals,
                                                              const Environment &mapping_bounds) {
    // generate deviation points randomly in the mapping region
    std::vector<XYZCoord> deviation_points;
    for (int i = 0; i < 200; i++) {
        deviation_points.push_back(mapping_bounds.getRandomPoint(true));
        printf("Deviation point: %f, %f\n", deviation_points.back().x, deviation_points.back().y);
    }

    // each deviation point can be inserted between any two goals
    std::vector<std::vector<XYZCoord>> new_goals_list;
    for (const XYZCoord &deviation_point : deviation_points) {
        std::vector<std::vector<XYZCoord>> goal_list_deviations =
            generateGoalListDeviations(goals, deviation_point);
        new_goals_list.insert(new_goals_list.end(), goal_list_deviations.begin(),
                              goal_list_deviations.end());
    }

    // run each goal list and get the area covered and the length of the path
    std::vector<std::pair<double, double>> area_length_pairs;
    for (const std::vector<XYZCoord> &new_goals : new_goals_list) {
        area_length_pairs.push_back(mapping_bounds.estimateAreaCoveredAndPathLength(new_goals));
    }

    // rank the new goal lists by the area covered and the length of the path
    std::vector<std::pair<double, std::vector<XYZCoord>>> ranked_new_goals_list;
    for (int i = 0; i < new_goals_list.size(); i++) {
        ranked_new_goals_list.push_back(
            {area_length_pairs[i].first / area_length_pairs[i].second, new_goals_list[i]});
    }

    std::sort(ranked_new_goals_list.begin(), ranked_new_goals_list.end(),
              [](const auto &a, const auto &b) { return a.first > b.first; });

    // return the ranked list of new goals lists
    std::vector<std::vector<XYZCoord>> ranked_goals;
    for (const auto &pair : ranked_new_goals_list) {
        ranked_goals.push_back(pair.second);
    }

    return ranked_goals;
}

/* TODO - doesn't match compeition spec 
   
    1. First waypoint is not home
    2. we omit the first waypoint (FATAL) - this means we never tell the computer to hit it
    3. We don't know where home location is - we rely on geofencing to not fly out of bounds
*/
MissionPath generateInitialPath(std::shared_ptr<MissionState> state) {
    // first waypoint is start

    // the other waypoitns is the goals
    if (state->mission_params.getWaypoints().size() < 2) {
        loguru::set_thread_name("Static Pathing");
        LOG_F(ERROR, "Not enough waypoints to generate a path, required 2+, existing waypoints: %s",
              std::to_string(state->mission_params.getWaypoints().size()).c_str());
        return {};
    }

    std::vector<XYZCoord> goals;

    // Copy elements from the second element to the last element of source into
    // destination all other methods of copying over crash???
    for (int i = 1; i < state->mission_params.getWaypoints().size(); i++) {
        goals.emplace_back(state->mission_params.getWaypoints()[i]);
    }

    // update goals here
    if (state->config.pathing.rrt.generate_deviations) {
        Environment mapping_bounds(state->mission_params.getMappingBoundary(), {}, {}, goals, {});
        goals = generateRankedNewGoalsList(goals, mapping_bounds)[0];
    }

    double init_angle =
        std::atan2(goals.front().y - state->mission_params.getWaypoints().front().y,
                   goals.front().x - state->mission_params.getWaypoints().front().x);
    RRTPoint start(state->mission_params.getWaypoints().front(), init_angle);
    start.coord.z = state->config.takeoff.altitude_m;

    RRT rrt(start, goals, SEARCH_RADIUS, state->mission_params.getFlightBoundary(), state->config,
            {}, {});

    rrt.run();

    std::vector<XYZCoord> path = rrt.getPointsToGoal();

    std::vector<GPSCoord> output_coords;
    for (const XYZCoord &wpt : goals) {
        output_coords.push_back(state->getCartesianConverter()->toLatLng(wpt));
    }

    return MissionPath(MissionPath::Type::FORWARD, output_coords);
}

MissionPath generateSearchPath(std::shared_ptr<MissionState> state) {
    std::vector<GPSCoord> gps_coords;
    if (state->config.pathing.coverage.method == AirdropCoverageMethod::Enum::FORWARD) {
        LOG_F(FATAL, "Forward search path not fully integrated yet.");

        RRTPoint start(state->mission_params.getWaypoints().front(), 0);

        // TODO , change the starting point to be something closer to loiter
        // region
        ForwardCoveragePathing pathing(start, SEARCH_RADIUS,
                                       state->mission_params.getFlightBoundary(),
                                       state->mission_params.getAirdropBoundary(), state->config);

        for (const auto &coord : pathing.run()) {
            gps_coords.push_back(state->getCartesianConverter()->toLatLng(coord));
        }

        return MissionPath(MissionPath::Type::FORWARD, {});
    } else {  // hover
        HoverCoveragePathing pathing(state);

        for (const auto &coord : pathing.run()) {
            gps_coords.push_back(state->getCartesianConverter()->toLatLng(coord));
        }
        return MissionPath(MissionPath::Type::HOVER, gps_coords,
                           state->config.pathing.coverage.hover.hover_time_s);
    }
}

MissionPath generateAirdropApproach(std::shared_ptr<MissionState> state, const GPSCoord &goal) {
    // finds starting location
    std::shared_ptr<MavlinkClient> mav = state->getMav();
    std::pair<double, double> start_lat_long = {38.315339, -76.548108};

    GPSCoord start_gps =
        makeGPSCoord(start_lat_long.first, start_lat_long.second, mav->altitude_agl_m());

    /*
        Note: this function was neutered right before we attempted to fly at the 2024 competition
        because we suddenly began running into an infinite loop during the execution of this
        function. Instead of spending an undeterministic amount of time to fix this problem,
        we ended up relying solely on Arduplane to navigate to the specified drop point
        instead of trying to formulate our own path.
    */

    double start_angle = 90 - mav->heading_deg();
    XYZCoord start_xyz = state->getCartesianConverter().value().toXYZ(start_gps);
    RRTPoint start_rrt(start_xyz, start_angle);

    // pathing
    XYZCoord goal_xyz = state->getCartesianConverter().value().toXYZ(goal);
    AirdropApproachPathing airdrop_planner(start_rrt, goal_xyz, mav->wind(),
                                           state->mission_params.getFlightBoundary(), state->config,
                                           {});
    std::vector<XYZCoord> xyz_path = airdrop_planner.run();

    // try to fly to the third waypoint in the path
    // prevents the drone from passing the initial waypoint
    // [TODO]-done out of laziness, forgot if the path includes starting location
    xyz_path.erase(xyz_path.begin());
    xyz_path.erase(xyz_path.begin());

    std::vector<GPSCoord> gps_path;
    // XYZCoord pt = state->getCartesianConverter().value().toXYZ(goal);

    for (const XYZCoord &wpt : xyz_path) {
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

    return MissionPath(MissionPath::Type::FORWARD, gps_path);
}
