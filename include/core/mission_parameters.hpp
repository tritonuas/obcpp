#ifndef INCLUDE_CORE_MISSION_PARAMETERS_HPP_
#define INCLUDE_CORE_MISSION_PARAMETERS_HPP_

#include <array>
#include <optional>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

// Make sure Airdrop type is known before it's used
#include "pathing/cartesian.hpp"
#include "protos/obc.pb.h"  // Include for Airdrop definition
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
    Polygon getMappingBoundary();
    Polyline getWaypoints();
    // CHANGE: Return type is now std::vector<Airdrop>
    std::vector<Airdrop> getAirdrops();

    // Getters for multiple values
    // Use when need to get multiple values
    // Important to use this instead of the singular getters
    // to avoid race conditions
    // CHANGE: Tuple element type is now std::vector<Airdrop>
    std::tuple<Polygon, Polygon, Polygon, Polyline, std::vector<Airdrop>> getConfig();

    // returns error string to be displayed back to the user
    // Ensure CartesianConverter<GPSProtoVec> template parameter matches usage
    std::optional<std::string> setMission(Mission, CartesianConverter<GPSProtoVec>);

    void saveToFile(std::string filename);

    std::optional<Mission> getCachedMission();

 private:
    std::shared_mutex mut;

    Polygon flightBoundary;
    Polygon airdropBoundary;
    Polygon mappingBoundary;
    Polyline waypoints;
    // CHANGE: Member variable type is now std::vector<Airdrop>
    std::vector<Airdrop> airdrops;

    std::optional<Mission> cached_mission{};

    // internal function which doesn't apply the mutex
    // CHANGE: Renamed function and changed parameter type to const Airdrop&
    void _setAirdrop(const Airdrop& airdrop);
};

#endif  // INCLUDE_CORE_MISSION_PARAMETERS_HPP_