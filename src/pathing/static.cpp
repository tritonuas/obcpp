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

std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state) {
    // first waypoint is start
    RRTPoint start(state->config.getWaypoints().front(), 0);

    // the other waypoitns is the goals
    if (state->config.getWaypoints().size() < 2) {
        std::cout << "Not enough waypoints" << std::endl;
        return {};
    }

    std::vector<XYZCoord> goals;

    // Copy elements (reference) from the second element to the last element of source into destination
    // all other methods of copying over crash???
    for (int i = 1; i < state->config.getWaypoints().size(); i++) {
        goals.emplace_back(state->config.getWaypoints()[i]);
    }

    RRT rrt(start, goals, ITERATIONS_PER_WAYPOINT, SEARCH_RADIUS, REWIRE_RADIUS,
            state->config.getFlightBoundary(), {},
            OptimizationOptions{true, PATH_OPTIONS::NONE});

    rrt.run();

    std::vector<XYZCoord> path = rrt.getPointsToGoal();
    std::vector<GPSCoord> output_coords;
    int count = 0;

    for (XYZCoord wpt : path) {
        output_coords.push_back(state->getCartesianConverter().value().toLatLng(wpt));
    }

    return output_coords;
}
