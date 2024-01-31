#ifndef INCLUDE_CORE_CONFIG_HPP_
#define INCLUDE_CORE_CONFIG_HPP_

#include <array>
#include <vector>
#include <string>
#include <tuple>
#include <shared_mutex>
#include <optional>
#include <unordered_map>

#include "utilities/datatypes.hpp"
#include "utilities/constants.hpp"

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

    // Check to see if the config has been initialized yet
    bool isValid() const;

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

    // Setters for singular value
    // Use when only need to update one value
    void setFlightBoundary(Polygon bound);
    void setAirdropBoundary(Polygon bound);
    void setWaypoints(Polyline wpts);
    // whatever index the bottle has, will replace that corresponding bottle in this config class
    void setBottle(Bottle bottle);
    // Update multiple bottles at a time
    void setBottles(const std::vector<Bottle>& bottleUpdates);

    // Use when need to update many things at once
    void batchUpdate(
        std::optional<Polygon> flight,
        std::optional<Polygon> airdrop,
        std::optional<Polyline> waypoints,
        std::vector<Bottle> bottleUpdates);

    void saveToFile(std::string filename);

 private:
    std::shared_mutex mut;

    Polygon flightBoundary;
    Polygon airdropBoundary;
    Polyline waypoints;
    std::vector<Bottle> bottles;

    // internal function which doesn't apply the mutex
    void _setBottle(Bottle bottle);
};

#endif  // INCLUDE_CORE_CONFIG_HPP_
