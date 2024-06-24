#ifndef INCLUDE_CORE_MISSION_PARAMETERS_HPP_
#define INCLUDE_CORE_MISSION_PARAMETERS_HPP_

#include <array>
#include <optional>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "pathing/cartesian.hpp"
#include "protos/obc.pb.h"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

/*
 *  Thread-safe wrapper around the Mission Configuration options.
 *  Multiple threads can access this same object, and it will handle
 *  all memory accesses with the shared mutex
 */
class MissionParameters {
 public:
    MissionParameters();                               // Load default values
    explicit MissionParameters(std::string filename);  // Load from filename

    // Getters for singular value
    // Use when only need to read one value

    Polygon getFlightBoundary();
    Polygon getAirdropBoundary();
    Polyline getWaypoints();
    std::vector<Bottle> getAirdropBottles();

    // Getters for multiple values
    // Use when need to get multiple values
    // Important to use this instead of the singular getters
    // to avoid race conditions
    std::tuple<Polygon, Polygon, Polyline, std::vector<Bottle>> getConfig();

    // returns error string to be displayed back to the user
    std::optional<std::string> setMission(Mission, CartesianConverter<GPSProtoVec>);

    void saveToFile(std::string filename);

    std::optional<Mission> getCachedMission();

 private:
    std::shared_mutex mut;

    Polygon flightBoundary;
    Polygon airdropBoundary;
    Polyline waypoints;
    std::vector<Bottle> bottles;

    std::optional<Mission> cached_mission{};

    // internal function which doesn't apply the mutex
    void _setBottle(Bottle bottle);
};

#endif  // INCLUDE_CORE_MISSION_PARAMETERS_HPP_
