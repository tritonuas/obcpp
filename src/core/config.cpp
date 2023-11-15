#include <cctype>
#include <iostream>
#include <shared_mutex>
#include <tuple>
#include <unordered_map>
#include <optional>

#include "core/config.hpp"
#include "utilities/constants.hpp"

MissionConfig::MissionConfig() {

}

MissionConfig::MissionConfig(std::string filename) {
    // TODO: load from file
}

Polygon MissionConfig::getFlightBoundary() {
    this->mut.lock_shared();
    auto bound = this->flightBoundary;
    this->mut.unlock_shared();
    return bound;
}

Polygon MissionConfig::getAirdropBoundary() {
    this->mut.lock_shared();
    auto bound = this->airdropBoundary;
    this->mut.unlock_shared();
    return bound;
}

Polyline MissionConfig::getWaypoints() {
    this->mut.lock_shared();
    auto bound = this->waypoints;
    this->mut.unlock_shared();
    return bound;
}

BottleArray MissionConfig::getAirdropBottles() {
    this->mut.lock_shared();
    auto bottles = this->bottles;
    this->mut.unlock_shared();
    return bottles;
} 

std::tuple<Polygon, Polygon, Polyline, BottleArray> MissionConfig::getConfig() {
    this->mut.lock_shared(); 
    auto tuple = std::make_tuple(this->flightBoundary, this->airdropBoundary, this->waypoints, this->bottles);
    this->mut.unlock_shared();
    return tuple;
}

void MissionConfig::setFlightBoundary(Polygon bound) {
    this->mut.lock();
    this->flightBoundary = bound;
    this->mut.unlock();
}

void MissionConfig::setAirdropBoundary(Polygon bound) {
    this->mut.lock();
    this->airdropBoundary = bound;
    this->mut.unlock();
}

void MissionConfig::setWaypoints(Polyline wpts) {
    this->mut.lock();
    this->waypoints = wpts;
    this->mut.unlock();
}

void MissionConfig::_setBottle(char label, CompetitionBottle bottle) {
    label = std::toupper(label);
    if (label < 'A' || label > 'F') {
        std::cerr << "Invalid bottle character " << label << " passed to MissionConfig::setBottle" << std::endl;
    }

    this->bottles[label - 'A'] = bottle;
}

void MissionConfig::setBottle(char label, CompetitionBottle bottle) {
    this->mut.lock();
    this->_setBottle(label, bottle);
    this->mut.unlock();
}

void MissionConfig::setBottles(const std::unordered_map<char, CompetitionBottle>& updates) {
    this->mut.lock();
    for (auto [label, bottle] : updates) {
        this->_setBottle(label, bottle);
    }
    this->mut.unlock();
}

void MissionConfig::update(
    std::optional<Polygon> flight,
    std::optional<Polygon> airdrop,
    std::optional<Polyline> waypoints,
    std::unordered_map<char, CompetitionBottle> bottleUpdates
) {
    this->mut.lock();

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

    this->mut.unlock();
}

void MissionConfig::saveToFile(std::string filename) {
    // TODO: implement
}