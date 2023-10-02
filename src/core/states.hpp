#ifndef CORE_STATES_HPP_
#define CORE_STATES_HPP_

#include <optional>
#include <string>
#include <array>

#include "datatypes.hpp"
#include "../utilities/constants.hpp"

/*
    Interface for an arbitrary mission state.
*/
class MissionState {
    public:
        /*
            Function that runs approx 1 time per second, doing the calculations/checks
            needed for the current phase of the mission.

            Returns the new MissionState if a state change needs to occur. If the optional
            type does not have a value, then no state change needs to occur. 
        */
        virtual MissionState* tick() = 0;

        /*
            Plain text name of the current state for display purposes 
        */
        virtual std::string getName() = 0;
};

/*
    State for when the system has just been turned on and is waiting for
    mission parameters.
*/
class PreparationState: public MissionState {
    public:
        MissionState* tick() override;

        std::string getName() override {
            return "Mission Preparation";
        }

    private:
        Polygon flightBoundary;
        Polygon airdropBoundary;
        Polyline waypoints;
        std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES> bottles;
};

#endif // CORE_STATES_HPP_