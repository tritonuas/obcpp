#ifndef CORE_CONFIG_HPP
#define CORE_CONFIG_HPP

#include <array>

#include "utilities/datatypes.hpp"
#include "utilities/constants.hpp"

class MissionConfig {
    public:
        MissionConfig();

        const Polygon& getFlightBoundary() const;        
        const Polygon& getAirdropBoundary() const;
        const Polyline& getWaypoints() const;
        const std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES>& getAirdropBottles() const; 

        void setFlightBoundary(Polygon bound);
        void setAirdropBoundary(Polygon bound);
        void setWaypoints(Polyline wpts);
        void setBottle(unsigned int index, CompetitionBottle);
    private:
        Polygon flightBoundary;
        Polygon airdropBoundary;
        Polyline waypoints;
        std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES> bottles;
};

#endif