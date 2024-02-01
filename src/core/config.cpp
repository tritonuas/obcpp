#include <cctype>
#include <iostream>
#include <shared_mutex>
#include <mutex>
#include <tuple>
#include <unordered_map>
#include <optional>

#include "core/config.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/locks.hpp"

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

bool MissionConfig::isValid() const {
    // 3 is minimum points that make sense for
    // a polygon to have
    return (this->airdropBoundary.size() >= 3 &&
            this->flightBoundary.size() >= 3 &&
            this->waypoints.size() > 0);
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

void MissionConfig::setFlightBoundary(Polygon bound) {
    WriteLock lock(this->mut);

    this->flightBoundary = bound;
}

void MissionConfig::setAirdropBoundary(Polygon bound) {
    WriteLock lock(this->mut);

    this->airdropBoundary = bound;
}

void MissionConfig::setWaypoints(Polyline wpts) {
    WriteLock lock(this->mut);

    this->waypoints = wpts;
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

void MissionConfig::setBottle(Bottle bottle) {
    WriteLock lock(this->mut);

    this->_setBottle(bottle);
}

void MissionConfig::setBottles(const std::vector<Bottle>& updates) {
    WriteLock lock(this->mut);

    for (auto bottle : updates) {
        this->_setBottle(bottle);
    }
}

void MissionConfig::batchUpdate(
    std::optional<Polygon> flight,
    std::optional<Polygon> airdrop,
    std::optional<Polyline> waypoints,
    std::vector<Bottle> bottleUpdates,
    Mission cached_mission
) {
    WriteLock lock(this->mut);

    this->cached_mission = cached_mission;

    if (flight.has_value()) {
        this->flightBoundary = flight.value();
    }

    if (airdrop.has_value()) {
        this->airdropBoundary = airdrop.value();
    }

    if (waypoints.has_value()) {
        this->waypoints = waypoints.value();
    }

    for (auto bottle : bottleUpdates) {
        this->_setBottle(bottle);
    }
}

void MissionConfig::saveToFile(std::string filename) {
    ReadLock lock(this->mut);

    // TODO: implement
}

std::optional<Mission> MissionConfig::getCachedMission() {
    ReadLock lock(this->mut);

    return this->cached_mission;
}
