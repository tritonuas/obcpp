#include "core/mission_parameters.hpp"

#include <cctype>
#include <iostream>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "pathing/cartesian.hpp"
#include "pathing/environment.hpp"
#include "protos/obc.pb.h"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/locks.hpp"

// TODO: Log out mission config after it has been set

MissionParameters::MissionParameters() {
    // Bottle updates work by finding the bottle already in the list
    // by index and setting its values to the updated values, so we
    // need to initialize placeholder values in the bottles vector
    Airdrop airdropA;
    airdropA.set_index(AirdropType::Water);
    Airdrop airdropB;
    airdropB.set_index(AirdropType::Beacon);
    // This part is now correct because this->airdrops is std::vector<Airdrop>
    this->airdrops.push_back(airdropA);
    this->airdrops.push_back(airdropB);
}

MissionParameters::MissionParameters(std::string filename) {
    // TODO: load from file
}

Polygon MissionParameters::getFlightBoundary() {
    ReadLock lock(this->mut);
    return this->flightBoundary;
}

Polygon MissionParameters::getAirdropBoundary() {
    ReadLock lock(this->mut);
    return this->airdropBoundary;
}

Polyline MissionParameters::getWaypoints() {
    ReadLock lock(this->mut);
    return this->waypoints;
}

std::vector<Airdrop> MissionParameters::getAirdrops() {
    ReadLock lock(this->mut);
    return this->airdrops;
}

std::tuple<Polygon, Polygon, Polyline, std::vector<Airdrop>>
MissionParameters::getConfig() {
    ReadLock lock(this->mut);

    return std::make_tuple(this->flightBoundary, this->airdropBoundary,
                           this->waypoints, this->airdrops);
}

void MissionParameters::_setAirdrop(const Airdrop& airdrop) {
    // Go until you find the airdrop that has the same index, and replace all values
    for (auto& curr_airdrop : this->airdrops) {  // existing_airdrop is Airdrop&
        // Compare index from the parameter (Airdrop object)
        if (curr_airdrop.index() == airdrop.index()) {
            // Assign the whole Airdrop object
            curr_airdrop = Airdrop(airdrop);
            break;
        }
    }
}

std::optional<std::string> MissionParameters::setMission(
    Mission mission, CartesianConverter<GPSProtoVec> cconverter) {
    WriteLock lock(this->mut);

    std::string err;
    if (mission.waypoints().empty()) {
        err += "Waypoints must have at least 1 waypoint. ";
    }

    if (mission.flightboundary().size() < 3) {
        err += "Flight boundary must have at least 3 coordinates. ";
    }

    if (mission.airdropboundary().size() < 3) {
        err += "Airdrop boundary must have at least 3 coordinates.";
    }

    if (!err.empty()) {
        return err;
    }

    const Polygon flight_boundary = cconverter.toXYZ(mission.flightboundary());
    const Polygon airdrop_boundary = cconverter.toXYZ(mission.airdropboundary());
    const Polyline mission_waypoints = cconverter.toXYZ(mission.waypoints());

    // nothing outside of the flight boundary can be flown to, so a mission that
    // asks for it is never going to be one the plane can fly
    for (std::size_t i = 0; i < mission_waypoints.size(); i++) {
        if (!Environment::isPointInPolygon(flight_boundary, mission_waypoints[i])) {
            err += "Waypoint " + std::to_string(i + 1) + " is outside of the flight boundary. ";
        }
    }

    // the airdrop boundary doubles as the mapping region
    if (!Environment::isPolygonInPolygon(airdrop_boundary, flight_boundary)) {
        err += "Airdrop boundary is not entirely inside of the flight boundary. ";
    }

    if (!err.empty()) {
        return err;
    }

    this->cached_mission = mission;
    this->flightBoundary = flight_boundary;
    this->airdropBoundary = airdrop_boundary;
    this->waypoints = mission_waypoints;
    for (const auto& airdrop : mission.airdropassignments()) {  // Use const& for efficiency
        this->_setAirdrop(airdrop);
    }

    Environment::init(this->flightBoundary, this->airdropBoundary, this->airdropBoundary);

    return {};
}

void MissionParameters::saveToFile(std::string filename) {
    ReadLock lock(this->mut);

    // TODO: implement
}

std::optional<Mission> MissionParameters::getCachedMission() {
    ReadLock lock(this->mut);

    return this->cached_mission;
}
