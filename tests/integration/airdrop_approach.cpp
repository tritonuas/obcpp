#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <thread>
#include <unordered_map>

#include "core/mission_state.hpp"
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

#include "handler_params.hpp"
#include <fstream>
#include <sstream>

/*
 * FILE OUTPUT LOCATIONS
 *  |- build
 *    |- pathing_output
 *      |- test_airdrop_approach.jpg
 *      |- test_airdrop_approach.gif (if enabled)
 *      |- approach_coords.txt
 *
 * This integration test 
 * 
 * 1. runs airdrop approach pathing on the 2024 mission,
 * 2. generates a plot,
 * 3. records coordinates for the path.
 * 4. Generate a gif if PahtOutType (STATIC, ANIMATED, BOTH)
 */
int main() {
    std::ofstream file;

    LOG_F(WARNING, "Airdrop Approach Pathing Testing");

    // Read mission data from JSON file
    std::ifstream mission_file("../tests/integration/util/mission_data_2024.json");
    if (!mission_file.is_open()) {
        LOG_F(ERROR, "Failed to open mission_data_2024.json");
        return 1;
    }
    
    std::stringstream buffer;
    buffer << mission_file.rdbuf();
    std::string mission_json_2024 = buffer.str();
    mission_file.close();

    // Upload the Mission
    DECLARE_HANDLER_PARAMS(state, req, resp);
    req.body = mission_json_2024;
    state->setTick(new MissionPrepTick(state));

    GCS_HANDLE(Post, mission)(state, req, resp);


    RRTPoint start = RRTPoint(state->mission_params.getWaypoints()[0], 0);
    XYZCoord target = XYZCoord(313.131212, -187.781235, 0);
    XYZCoord wind = XYZCoord(0, 0, 0);

    AirdropApproachPathing approach(start, target, wind,
                             state->mission_params.getFlightBoundary(), state->config);

    LOG_F(WARNING, "Running Approach");
    std::vector<XYZCoord> path = approach.run();
    LOG_F(INFO, "Path size: %zu", path.size());

    // files to put path_coordinates to
    file.open("pathing_output/approach_coords.txt");
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
                        state->mission_params.getAirdropBoundary(), {});

    plotter.addFinalPolyline(path);
    plotter.output("test_airdrop_approach", PathOutputType::STATIC);
    return 0;
}