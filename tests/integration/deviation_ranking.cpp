#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <thread>
#include <unordered_map>

#include "core/mission_state.hpp"
#include "handler_params.hpp"
#include "network/gcs.hpp"
#include "network/gcs_macros.hpp"
#include "network/gcs_routes.hpp"
#include "pathing/plotting.hpp"
#include "pathing/static.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/mission_prep.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/tick.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/http.hpp"

/*
 * FILE OUTPUT LOCATIONS
 *  |-- build
 *      |-- pathing_output
 *          |-- deviation.jpg
 *          |-- deviation_path.gif (if enabled)
 *          |-- deviation_coordinates.txt
 *
 *  This rough integration test is to test the airdrop search pathing algorithm
 */
int main() {
    std::ofstream file;

    LOG_F(WARNING, "Deviation Ranking Testing");

    // Read mission data from JSON file
    std::ifstream mission_file("../tests/integration/util/mission_data_2020.json");
    if (!mission_file.is_open()) {
        LOG_F(ERROR, "Failed to open mission_data_2020.json");
        return 1;
    }
    
    std::stringstream buffer;
    buffer << mission_file.rdbuf();
    std::string mission_json_2020 = buffer.str();
    mission_file.close();

    // Upload the Mission
    DECLARE_HANDLER_PARAMS(state, req, resp);
    req.body = mission_json_2020;
    state->setTick(new MissionPrepTick(state));

    // create the environment with custome mapping region
    Polygon mapping_region = {XYZCoord(-200, 150, 0), XYZCoord(100, 75, 0), XYZCoord(-125, 300, 0),
                              XYZCoord(-300, 300, 0)};
    Environment env(state->mission_params.getFlightBoundary(),
                    state->mission_params.getAirdropBoundary(), mapping_region, {}, {});
    std::vector<XYZCoord> goals = state->mission_params.getWaypoints();

    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<XYZCoord>> rank_new_goals_list = generateRankedNewGoalsList(goals, env);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    LOG_F(INFO, "Time to run: %f s", elapsed.count());

    std::vector<XYZCoord> new_goals = rank_new_goals_list[0];
    RRTPoint start = RRTPoint(new_goals[0], 0);
    start.coord.z = 100;
    new_goals.erase(new_goals.begin());

    double search_radius = 9999;

    RRT rrt = RRT(start, new_goals, search_radius, state->mission_params.getFlightBoundary(),
                  state->config, {}, {});

    // run the rrt algorithm
    rrt.run();

    // get the path, put it into the file
    std::vector<XYZCoord> path = rrt.getPointsToGoal();
    LOG_F(INFO, "Path size: %d", path.size());
    LOG_F(INFO, "Path length: %f", path.size() * state->config.pathing.dubins.point_separation);

    // files to put path_coordinates to
    file.open("pathing_output/deviation_coordinates.txt");
    for (const XYZCoord& point : path) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) 
            << std::setw(12) << point.x << ", "
            << std::setw(12) << point.y << ", "
            << std::setw(12) << point.z << '\n';
        file << oss.str();
    }
    file.close();

    // plot the path
    PathingPlot plotter("pathing_output", state->mission_params.getFlightBoundary(), 
      mapping_region, new_goals);
    // plotter.addFinalPolyline(path);
    plotter.output("deviation_path", PathOutputType::STATIC);

    return 0;
}