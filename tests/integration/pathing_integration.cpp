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

const static char* mission_json_2020 = R"(
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
        {
          "Latitude": 38.1462694444444,
          "Longitude": -76.4281638888889
        },
        {
          "Latitude": 38.151625,
          "Longitude": -76.4286833333333
        },
        {
          "Latitude": 38.1518888888889,
          "Longitude": -76.4314666666667
        },
        {
          "Latitude": 38.1505944444444,
          "Longitude": -76.4353611111111
        },
        {
          "Latitude": 38.1475666666667,
          "Longitude": -76.4323416666667
        },
        {
          "Latitude": 38.1446666666667,
          "Longitude": -76.4329472222222
        },
        {
          "Latitude": 38.1432555555556,
          "Longitude": -76.4347666666667
        },
        {
          "Latitude": 38.1404638888889,
          "Longitude": -76.4326361111111
        },
        {
          "Latitude": 38.1407194444444,
          "Longitude": -76.4260138888889
        },
        {
          "Latitude": 38.1437611111111,
          "Longitude": -76.4212055555556
        },
        {
          "Latitude": 38.1473472222222,
          "Longitude": -76.4232111111111
        },
        {
          "Latitude": 38.1461305555556,
          "Longitude": -76.4266527777778
        }
    ],
  "AirdropBoundary": [
    {
      "Latitude": 38.1444444444444,
      "Longitude": -76.4280916666667
    },
    {
      "Latitude": 38.1459444444444,
      "Longitude": -76.4237944444445
    },
    {
      "Latitude": 38.1439305555556,
      "Longitude": -76.4227444444444
    },
    {
      "Latitude": 38.1417138888889,
      "Longitude": -76.4253805555556
    },
    {
      "Latitude": 38.1412111111111,
      "Longitude": -76.4322361111111
    },
    {
      "Latitude": 38.1431055555556,
      "Longitude": -76.4335972222222
    },
    {
      "Latitude": 38.1441805555556,
      "Longitude": -76.4320111111111
    },
    {
      "Latitude": 38.1452611111111,
      "Longitude": -76.4289194444444
    },
    {
      "Latitude": 38.1444444444444,
      "Longitude": -76.4280916666667
    }
  ],
  "Waypoints": [
    {
      "Latitude": 38.1446916666667,
      "Longitude": -76.4279944444445,
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
 *      |- test_final_path.jpg
 *      |- test_final_path.gif (if enabled)
 *    |- path_coordinates.txt
 *
 * This integration test runs static path finding once, and generates a plot as
 * well as returns the coordinates for the path.
 */
int main() {
    std::cout << "Messing with RRT*" << std::endl;
    // First upload a mission so that we generate a path
    // this is roughly the mission from 2020
    DECLARE_HANDLER_PARAMS(state, req, resp);
    req.body = mission_json_2020;
    state->setTick(new MissionPrepTick(state));
    std::cout << state->config.rrt.iterations_per_waypoint << std::endl;

    GCS_HANDLE(Post, mission)(state, req, resp);

    // files to put path_coordinates to
    std::ofstream file;
    file.open("path_coordinates.txt");

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

    // runs the pathing 
    state->doTick();

    std::vector<GPSCoord> gps_path = state->getInitPath();

    std::vector<XYZCoord> path;

    for (GPSCoord &point : gps_path) {
        path.push_back(state->getCartesianConverter().value().toXYZ(point));
    }

    // get the path, put it into the file
    std::cout << "Path size: " << path.size() << std::endl;
    std::cout << "Path length: " << (path.size() * POINT_SEPARATION) << std::endl;
    for (const XYZCoord& point : path) {
        file << point.x << ", " << point.y << std::endl;
    }

    // plot the path
    std::cout << "Start Plotting" << std::endl;
    PathingPlot plotter("pathing_output", state->mission_params.getFlightBoundary(), obstacles[1], goals);

    plotter.addFinalPolyline(path);
    plotter.output("test_final_path", PathOutputType::STATIC);

    file.close();
    return 0;
}