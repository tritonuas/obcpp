#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>
#include <unordered_map>

#include "core/mission_state.hpp"
#include "network/gcs.hpp"
#include "network/gcs_macros.hpp"
#include "network/gcs_routes.hpp"
#include "pathing/plotting.hpp"
#include "pathing/static.hpp"
#include "ticks/mission_prep.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/tick.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/http.hpp"

#include "handler_params.hpp"

/*
 * FILE OUTPUT LOCATIONS
 *  |- build
 *    |- pathing_output
 *      |- test_final_path.jpg
 *      |- test_final_path.gif (if enabled)
 *      |- path_coordinates.txt
 *
 * This integration test 
 * 
 * 1. runs static path finding once on the 2020 mission,
 * 2. generates a plot,
 * 3. records coordinates for the path.
 * 
 * Ideal Path Length: ~5600
 */
int main() {
    std::ofstream file;

    LOG_F(WARNING, "RRT Waypoint Testing");

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

    GCS_HANDLE(Post, mission)(state, req, resp);

    // infrastructure to set up all the pararmeters of the environment
    std::vector<XYZCoord> goals;

    for (const XYZCoord& waypoint : state->mission_params.getWaypoints()) {
        goals.push_back(waypoint);
    }

    goals.erase(goals.begin());

    Polygon obs1 = {XYZCoord(-200, 150, 0), XYZCoord(100, 75, 0), XYZCoord(-125, 300, 0),
                    XYZCoord(-300, 300, 0)};

    Polygon obs2 = {XYZCoord(-200, -600, 0), XYZCoord(100, -600, 0), XYZCoord(250, -300, 0),
                    XYZCoord(0, -300, 0)};

    std::vector<Polygon> obstacles = {obs1, obs2};

    RRTPoint start = RRTPoint(state->mission_params.getWaypoints()[0], 0);
    start.coord.z = 30.0; // 30 meters takeoff

    // RRT settings (manually put in)
    double search_radius = 9999;
    LOG_F(WARNING, "RRT Stats");
    LOG_F(INFO, "Search Radius %f", search_radius);

    RRT rrt = RRT(start, goals, search_radius,  
                  state->mission_params.getFlightBoundary(), state->config, obstacles, {});

    //run the algoritm, and time it
    auto start_time = std::chrono::high_resolution_clock::now();
    rrt.run();
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    LOG_F(INFO, "Time to run: %f s", elapsed.count());

    // get the path, put it into the file
    std::vector<XYZCoord> path = rrt.getPointsToGoal();
    LOG_F(INFO, "Path size: %d", path.size());
    LOG_F(INFO, "Path length: %f", path.size() * state->config.pathing.dubins.point_separation);

    // files to put path_coordinates to
    file.open("pathing_output/path_coordinates.txt");
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
    PathingPlot plotter("pathing_output", state->mission_params.getFlightBoundary(), obstacles[1], goals);
    plotter.addFinalPolyline(path);
    plotter.output("test_final_path", PathOutputType::STATIC);

    return 0;
}