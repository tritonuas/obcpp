#include "pathing/static.hpp"

std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state) {
    // first waypoint is start
    RRTPoint start(state->config.getWaypoints().front(), 0);

    // the other waypoitns is the goals
    if (state->config.getWaypoints().size() < 2) {
        std::cout << "Not enough waypoints" << std::endl;
        return {};
    }

    std::vector<XYZCoord> goals;

    // Copy elements from the second element to the last element of source into destination
    // all other methods of copying over crash???
    for (int i = 1; i < state->config.getWaypoints().size(); i++) {
        goals.push_back(state->config.getWaypoints()[i]);
    }

    // TODO remove magic numbers
    RRT rrt(start, goals, 1500, 1000, 200, state->config.getFlightBoundary(), {}, true);
    rrt.run();

    std::vector<XYZCoord> path = rrt.getPointsToGoal();
    std::vector<GPSCoord> output_coords;
    int count = 0;

    for (XYZCoord wpt : path) {
        output_coords.push_back(state->getCartesianConverter().value().toLatLng(wpt));
    }

    return output_coords;
}