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
#include "ticks/mission_prep.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/tick.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/http.hpp"

#define DECLARE_HANDLER_PARAMS(STATE, REQ, RESP)                            \
    std::shared_ptr<MissionState> STATE = std::make_shared<MissionState>(); \
    httplib::Request REQ;                                                   \
    httplib::Response RESP



const static char* mission_json_2024 = R"(
{
    "BottleAssignments": [
        {
            "Alphanumeric": "",
            "AlphanumericColor": 0,
            "Shape": 0,
            "ShapeColor": 0,
            "Index": 1,
            "IsMannikin": true
        },
        {
            "Alphanumeric": "",
            "AlphanumericColor": 0,
            "Shape": 0,
            "ShapeColor": 0,
            "Index": 2,
            "IsMannikin": true
        },
        {
            "Alphanumeric": "",
            "AlphanumericColor": 0,
            "Shape": 0,
            "ShapeColor": 0,
            "Index": 3,
            "IsMannikin": true
        },
        {
            "Alphanumeric": "",
            "AlphanumericColor": 0,
            "Shape": 0,
            "ShapeColor": 0,
            "Index": 4,
            "IsMannikin": true
        },
        {
            "Alphanumeric": "",
            "AlphanumericColor": 0,
            "Shape": 0,
            "ShapeColor": 0,
            "Index": 5,
            "IsMannikin": true
        }
    ],
  "FlightBoundary": [
  {"Latitude": 38.31729702009844, "Longitude": -76.55617670782419},
  {"Latitude": 38.31594832826572, "Longitude": -76.55657341657302},
  {"Latitude": 38.31546739500083, "Longitude": -76.55376201277696},
  {"Latitude": 38.31470980862425, "Longitude": -76.54936361414539},
  {"Latitude": 38.31424154692598, "Longitude": -76.54662761646904},
  {"Latitude": 38.31369801280048, "Longitude": -76.54342380058223},
  {"Latitude": 38.31331079191371, "Longitude": -76.54109648475954},
  {"Latitude": 38.31529941346197, "Longitude": -76.54052104837133},
  {"Latitude": 38.31587643291039, "Longitude": -76.54361305817427},
  {"Latitude": 38.31861642463319, "Longitude": -76.54538594175376},
  {"Latitude": 38.31862683616554, "Longitude": -76.55206138505936},
  {"Latitude": 38.31703471119464, "Longitude": -76.55244787859773},
  {"Latitude": 38.31674255749409, "Longitude": -76.55294546866578},
  {"Latitude": 38.31729702009844, "Longitude": -76.55617670782419}
],
  "AirdropBoundary": [
  {"Latitude": 38.31442311312976, "Longitude": -76.54522971451763},
  {"Latitude": 38.31421041772561, "Longitude": -76.54400246436776},
  {"Latitude": 38.3144070396263, "Longitude": -76.54394394383165},
  {"Latitude": 38.31461622313521, "Longitude": -76.54516993186949},
  {"Latitude": 38.31442311312976, "Longitude": -76.54522971451763}
],
  "Waypoints": [
    {
      "Latitude": 38.31442311312976, 
      "Longitude": -76.54522971451763,
      "Altitude": 200.0
    },
    {
      "Latitude": 38.1461944444444,
      "Longitude": -76.4237138888889,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1438972222222,
      "Longitude": -76.42255,
      "Altitude": 400.0
    },
    {
      "Latitude": 38.1417722222222,
      "Longitude": -76.4251083333333,
      "Altitude": 400.0
    },
    {
      "Latitude": 38.14535,
      "Longitude": -76.428675,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1508972222222,
      "Longitude": -76.4292972222222,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1514944444444,
      "Longitude": -76.4313833333333,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1505333333333,
      "Longitude": -76.434175,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1479472222222,
      "Longitude": -76.4316055555556,
      "Altitude": 200.0
    },
    {
      "Latitude": 38.1443333333333,
      "Longitude": -76.4322888888889,
      "Altitude": 200.0
    },
    {
      "Latitude": 38.1433166666667,
      "Longitude": -76.4337111111111,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1410944444444,
      "Longitude": -76.4321555555556,
      "Altitude": 400.0
    },
    {
      "Latitude": 38.1415777777778,
      "Longitude": -76.4252472222222,
      "Altitude": 400.0
    },
    {
      "Latitude": 38.1446083333333,
      "Longitude": -76.4282527777778,
      "Altitude": 200.0
    }
  ]
})";

/*
 * FILE OUTPUT LOCATIONS
 *  |- build
 *    |- pathing_output
 *      |- test_airdrop_pathing.jpg
 *      |- test_airdrop_pathing.gif (if enabled)
 *    |- airdop_search_coords.txt
 *
 *  This rough integration test is to test the airdrop search pathing algorithm
 */
int main() {
    std::cout << "Messing with Airdrop Zone Search Pathing Part 2" << std::endl;
    // First upload a mission so that we generate a path
    // this is roughly the mission from 2020
    DECLARE_HANDLER_PARAMS(state, req, resp);
    req.body = mission_json_2024;
    state->setTick(new MissionPrepTick(state));

    GCS_HANDLE(Post, mission)(state, req, resp);

    // files to put path_coordinates to
    std::ofstream file;
    file.open("airdop_search_coords_2.txt");

    RRTPoint start = RRTPoint(state->config.getWaypoints()[0], 0);

    AirdropSearch search(start, 10, state->config.getFlightBoundary(),
                         state->config.getAirdropBoundary());

    std::vector<XYZCoord> path = search.run();

    // plot the path
    std::cout << "Start Plotting" << std::endl;
    PathingPlot plotter("pathing_output", state->config.getFlightBoundary(),
                        state->config.getAirdropBoundary(), {});

    plotter.addFinalPolyline(path);
    plotter.output("test_airdrop_pathing_2", PathOutputType::STATIC);
    file.close();
    return 0;
}