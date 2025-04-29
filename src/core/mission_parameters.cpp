#include "core/mission_parameters.hpp"

#include <cctype>
#include <iostream>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <tuple>
#include <unordered_map>

#include "pathing/cartesian.hpp"
#include "protos/obc.pb.h"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/locks.hpp"

// TODO: Log out mission config after it has been set

MissionParameters::MissionParameters() {
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

Polygon MissionParameters::getMappingBoundary() {
    ReadLock lock(this->mut);

    return this->mappingBoundary;
}

Polyline MissionParameters::getWaypoints() {
    ReadLock lock(this->mut);

    return this->waypoints;
}

std::vector<Bottle> MissionParameters::getAirdropBottles() {
    ReadLock lock(this->mut);

    return this->bottles;
}

std::tuple<Polygon, Polygon, Polyline, std::vector<Bottle>> MissionParameters::getConfig() {
    ReadLock lock(this->mut);

    return std::make_tuple(this->flightBoundary, this->airdropBoundary, this->mappingBoundary,
                           this->waypoints, this->bottles);
}

void MissionParameters::_setBottle(Bottle bottle) {
    // Go until you find the bottle that has the same index, and replace all values
    for (auto& curr_bottle : this->bottles) {
        if (curr_bottle.index() == bottle.index()) {
            curr_bottle = Bottle(bottle);
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

    if (mission.mappingboundary().size() < 3) {
        err += "Mapping boundary must have at least 3 coordinates.";
    }

    if (!err.empty()) {
        return err;
    }

    this->cached_mission = mission;
    this->flightBoundary = cconverter.toXYZ(mission.flightboundary());
    this->airdropBoundary = cconverter.toXYZ(mission.airdropboundary());
    this->mappingBoundary = cconverter.toXYZ(mission.mappingboundary());
    this->waypoints = cconverter.toXYZ(mission.waypoints());
    for (auto bottle : mission.bottleassignments()) {
        this->_setBottle(bottle);
    }

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
