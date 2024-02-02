#include <cctype>
#include <iostream>
#include <shared_mutex>
#include <mutex>
#include <tuple>
#include <unordered_map>
#include <optional>

#include "core/mission_config.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/locks.hpp"
#include "pathing/cartesian.hpp"

#include "protos/obc.pb.h"

// TODO: Log out mission config after it has been set

MissionConfig::MissionConfig() {
    // Bottle updates work by finding the bottle already in the list
    // by index and setting its values to the updated values, so we
    // need to initialize placeholder values in the bottles vector
    Bottle bottleA;
    bottleA.set_index(BottleDropIndex::A);
    Bottle bottleB;
    bottleB.set_index(BottleDropIndex::B);
    Bottle bottleC;
    bottleC.set_index(BottleDropIndex::C);
    Bottle bottleD;
    bottleD.set_index(BottleDropIndex::D);
    Bottle bottleE;
    bottleE.set_index(BottleDropIndex::E);
    this->bottles.push_back(bottleA);
    this->bottles.push_back(bottleB);
    this->bottles.push_back(bottleC);
    this->bottles.push_back(bottleD);
    this->bottles.push_back(bottleE);
}

MissionConfig::MissionConfig(std::string filename) {
    // TODO: load from file
}

Polygon MissionConfig::getFlightBoundary() {
    ReadLock lock(this->mut);

    return this->flightBoundary;
}

Polygon MissionConfig::getAirdropBoundary() {
    ReadLock lock(this->mut);

    return this->airdropBoundary;
}

Polyline MissionConfig::getWaypoints() {
    ReadLock lock(this->mut);

    return this->waypoints;
}

const std::vector<Bottle>& MissionConfig::getAirdropBottles() {
    ReadLock lock(this->mut);

    return this->bottles;
}

std::tuple<Polygon, Polygon, Polyline, std::vector<Bottle>> MissionConfig::getConfig() {
    ReadLock lock(this->mut);

    return std::make_tuple(
        this->flightBoundary, this->airdropBoundary, this->waypoints, this->bottles);
}

void MissionConfig::_setBottle(Bottle bottle) {
    // Go until you find the bottle that has the same index, and replace all values
    for (auto& curr_bottle : this->bottles) {
        if (curr_bottle.index() == bottle.index()) {
            curr_bottle = Bottle(bottle);
            break;
        }
    }
}


std::optional<std::string> MissionConfig::setMission(
    Mission mission, CartesianConverter<GPSProtoVec> cconverter
) {
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

    this->cached_mission = mission;
    this->flightBoundary = cconverter.toXYZ(mission.flightboundary());
    this->airdropBoundary = cconverter.toXYZ(mission.airdropboundary());
    this->waypoints = cconverter.toXYZ(mission.waypoints());
    for (auto bottle : mission.bottleassignments()) {
        this->_setBottle(bottle);
    }

    return {};
}

void MissionConfig::saveToFile(std::string filename) {
    ReadLock lock(this->mut);

    // TODO: implement
}

std::optional<Mission> MissionConfig::getCachedMission() {
    ReadLock lock(this->mut);

    return this->cached_mission;
}
