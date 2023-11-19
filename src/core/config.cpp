#include <cctype>
#include <iostream>
#include <shared_mutex>
#include <mutex>
#include <tuple>
#include <unordered_map>
#include <optional>

#include "core/config.hpp"
#include "utilities/constants.hpp"
#include "utilities/locks.hpp"


MissionConfig::MissionConfig():
    flightBoundary(FLIGHT_BOUND_COLOR),
    airdropBoundary(AIRDROP_BOUND_COLOR),
    waypoints(WAYPOINTS_COLOR) 
{
        
}

MissionConfig::MissionConfig(std::string filename):
    flightBoundary(FLIGHT_BOUND_COLOR),
    airdropBoundary(AIRDROP_BOUND_COLOR),
    waypoints(WAYPOINTS_COLOR) 
{
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

BottleArray MissionConfig::getAirdropBottles() {
    ReadLock lock(this->mut);

    return this->bottles;
} 

std::tuple<Polygon, Polygon, Polyline, BottleArray> MissionConfig::getConfig() {
    ReadLock lock(this->mut);

    return std::make_tuple(this->flightBoundary, this->airdropBoundary, this->waypoints, this->bottles);
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

void MissionConfig::_setBottle(char label, CompetitionBottle bottle) {
    label = std::toupper(label);
    if (label < 'A' || label > 'F') {
        std::cerr << "Invalid bottle character " << label << " passed to MissionConfig::setBottle" << std::endl;
    }

    this->bottles[label - 'A'] = bottle;
}

void MissionConfig::setBottle(char label, CompetitionBottle bottle) {
    WriteLock lock(this->mut);

    this->_setBottle(label, bottle);
}

void MissionConfig::setBottles(const std::unordered_map<char, CompetitionBottle>& updates) {
    WriteLock lock(this->mut);

    for (auto [label, bottle] : updates) {
        this->_setBottle(label, bottle);
    }
}

void MissionConfig::batchUpdate(
    std::optional<Polygon> flight,
    std::optional<Polygon> airdrop,
    std::optional<Polyline> waypoints,
    std::unordered_map<char, CompetitionBottle> bottleUpdates
) {
    WriteLock lock(this->mut);

    if (flight.has_value()) {
        this->flightBoundary = flight.value();
    }

    if (airdrop.has_value()) {
        this->airdropBoundary = airdrop.value();
    }

    if (waypoints.has_value()) {
        this->waypoints = waypoints.value();
    }

    for (auto [label, bottle] : bottleUpdates) {
        this->_setBottle(label, bottle);
    }
}

void MissionConfig::saveToFile(std::string filename) {
    ReadLock lock(this->mut);

    // TODO: implement
}