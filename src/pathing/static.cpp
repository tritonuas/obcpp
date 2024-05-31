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
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

RRT::RRT(RRTPoint start, std::vector<XYZCoord> goals, double search_radius, Polygon bounds,
         std::vector<Polygon> obstacles, std::vector<double> angles, RRTConfig config)
    : iterations_per_waypoint(config.iterations_per_waypoint),
      search_radius(search_radius),
      rewire_radius(config.rewire_radius),
      tree(start, Environment(bounds, {}, goals, obstacles),
           Dubins(TURNING_RADIUS, POINT_SEPARATION)),
      config(config) {
    if (angles.size() != 0) {
        this->angles = angles;
    }
}

RRT::RRT(RRTPoint start, std::vector<XYZCoord> goals, double search_radius, Environment airspace,
         std::vector<double> angles, RRTConfig config)
    : iterations_per_waypoint(config.iterations_per_waypoint),
      search_radius(search_radius),
      rewire_radius(config.rewire_radius),
      tree(start, airspace, Dubins(TURNING_RADIUS, POINT_SEPARATION)),
      config(config) {
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
            tree.pathingOptions(goal, POINT_FETCH_METHODS::NONE, NUMBER_OPTIONS_EACH);

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

CoveragePathing::CoveragePathing(const RRTPoint &start, double scan_radius, Polygon bounds,
                                 Polygon airdrop_zone, std::vector<Polygon> obstacles,
                                 AirdropSearchConfig config)
    : start(start),
      scan_radius(scan_radius),
      airspace(Environment(bounds, airdrop_zone, {}, obstacles)),
      dubins(Dubins(TURNING_RADIUS, POINT_SEPARATION)),
      config(config) {}

std::vector<XYZCoord> CoveragePathing::run() const {
    return config.optimize ? coverageOptimal() : coverageDefault();
}

std::vector<XYZCoord> CoveragePathing::coverageDefault() const {
    // generates the endpoints for the lines (including headings)
    std::vector<RRTPoint> waypoints =
        airspace.getAirdropWaypoints(scan_radius, config.one_way, config.vertical);
    waypoints.emplace(waypoints.begin(), start);

    // generates the path connecting the q
    std::vector<RRTOption> dubins_options;
    for (int i = 0; i < waypoints.size() - 1; i++) {
        dubins_options.push_back(dubins.bestOption(waypoints[i], waypoints[i + 1]));
    }

    return generatePath(dubins_options, waypoints);
}

std::vector<XYZCoord> CoveragePathing::coverageOptimal() const {
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

std::vector<XYZCoord> CoveragePathing::generatePath(const std::vector<RRTOption> &dubins_options,
                                                    const std::vector<RRTPoint> &waypoints) const {
    std::vector<XYZCoord> path;

    // height adjustement
    double height = waypoints[0].coord.z;
    double height_difference = config.coverage_altitude_m - waypoints[0].coord.z;

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
            coord.z = config.coverage_altitude_m;
        }

        path.insert(path.end(), path_coordinates.begin() + 1, path_coordinates.end());
    }

    return path;
}

AirdropApproachPathing::AirdropApproachPathing(const RRTPoint &start, const XYZCoord &goal,
                                               RRTPoint wind, Polygon bounds,
                                               std::vector<Polygon> obstacles,
                                               AirdropApproachConfig config)
    : start(start),
      goal(goal),
      wind(wind),
      airspace(Environment(bounds, {}, {goal}, obstacles)),
      dubins(Dubins(TURNING_RADIUS, POINT_SEPARATION)),
      config(config) {}

std::vector<XYZCoord> AirdropApproachPathing::run() const {
    RRTPoint drop_vector = getDropLocation();
    RRT rrt(start, {drop_vector.coord}, SEARCH_RADIUS, airspace, {drop_vector.psi});
    rrt.run();

    return rrt.getPointsToGoal();
}

RRTPoint AirdropApproachPathing::getDropLocation() const {
    double drop_angle = config.drop_angle_rad;
    double drop_distance = config.drop_method == GUIDED ? config.unguided_drop_distance_m
                                                        : config.guided_drop_distance_m;

    XYZCoord drop_offset(drop_distance * std::cos(drop_angle), drop_distance * std::sin(drop_angle),
                         0);

    double wind_strength_coef = wind.coord.norm() * WIND_CONST_PER_ALTITUDE;
    XYZCoord wind_offset(wind_strength_coef * std::cos(wind.psi),
                         wind_strength_coef * std::sin(wind.psi), 0);
    XYZCoord drop_location(goal.x + drop_offset.x + wind_offset.x,
                           goal.y + drop_offset.y + wind_offset.y, config.drop_altitude_m);

    // gets the angle between the drop_location and the goal
    double angle = std::atan2(goal.y - drop_location.y, goal.x - drop_location.x);
    return RRTPoint(drop_location, angle);
}

std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state) {
    // first waypoint is start

    // the other waypoitns is the goals
    if (state->mission_params.getWaypoints().size() < 2) {
        loguru::set_thread_name("Static Pathing");
        LOG_F(ERROR, "Not enough waypoints to generate a path, required 2+, existing waypoints: %s",
              std::to_string(state->mission_params.getWaypoints().size()).c_str());
        return {};
    }

    std::vector<XYZCoord> goals;

    // Copy elements (reference) from the second element to the last element of source into
    // destination all other methods of copying over crash???
    for (int i = 1; i < state->mission_params.getWaypoints().size(); i++) {
        goals.emplace_back(state->mission_params.getWaypoints()[i]);
    }

    double init_angle =
        std::atan2(goals.front().y - state->mission_params.getWaypoints().front().y,
                   goals.front().x - state->mission_params.getWaypoints().front().x);
    RRTPoint start(state->mission_params.getWaypoints().front(), init_angle);
    start.coord.z = state->takeoff_alt_m;

    RRT rrt(start, goals, SEARCH_RADIUS, state->mission_params.getFlightBoundary(), {}, {},
            state->rrt_config);

    rrt.run();

    std::vector<XYZCoord> path = rrt.getPointsToGoal();
    std::vector<GPSCoord> output_coords;

    for (const XYZCoord& wpt : path) {
        output_coords.push_back(state->getCartesianConverter().value().toLatLng(wpt));
    }

    return output_coords;
}

std::vector<GPSCoord> generateAirdropApproach(std::shared_ptr<MissionState> state,
                                              const GPSCoord &goal) {
    std::shared_ptr<MavlinkClient> mav = state->getMav();
    std::pair<double, double> lat_long_start = mav->latlng_deg();
    GPSCoord gps_start =
        makeGPSCoord(lat_long_start.first, lat_long_start.second, mav->altitude_agl_m());
    double start_angle = 90 - mav->heading_deg();
    XYZCoord xyz_start = state->getCartesianConverter().value().toXYZ(gps_start);
    RRTPoint start(xyz_start, start_angle);

    XYZCoord xyz_goal = state->getCartesianConverter().value().toXYZ(goal);

    
    // DO WIND
    AirdropApproachPathing airdrop_planner(start, xyz_goal, RRTPoint(XYZCoord(0,0,0), 0), state->mission_params.getFlightBoundary());

    std::vector<XYZCoord> path = airdrop_planner.run();
    std::vector<GPSCoord> output_coords;

    for (const XYZCoord& wpt : path) {
        output_coords.push_back(state->getCartesianConverter().value().toLatLng(wpt));
    }

    return output_coords;
}
