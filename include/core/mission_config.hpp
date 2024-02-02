#ifndef INCLUDE_CORE_MISSION_CONFIG_HPP_
#define INCLUDE_CORE_MISSION_CONFIG_HPP_

#include <array>
#include <vector>
#include <string>
#include <tuple>
#include <shared_mutex>
#include <optional>
#include <unordered_map>

#include "utilities/datatypes.hpp"
#include "utilities/constants.hpp"
#include "pathing/cartesian.hpp"

#include "protos/obc.pb.h"

/*
 *  Thread-safe wrapper around the Mission Configuration options.
 *  Multiple threads can access this same object, and it will handle
 *  all memory accesses with the shared mutex
 */
class MissionConfig {
 public:
    MissionConfig();  // Load default values
    explicit MissionConfig(std::string filename);  // Load from filename

    // Getters for singular value
    // Use when only need to read one value
    Polygon getFlightBoundary();
    Polygon getAirdropBoundary();
    Polyline getWaypoints();
    const std::vector<Bottle>& getAirdropBottles();

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

    std::optional<Mission> cached_mission {};

    // internal function which doesn't apply the mutex
    void _setBottle(Bottle bottle);
};

#endif  // INCLUDE_CORE_MISSION_CONFIG_HPP_
