#ifndef INCLUDE_CORE_STATES_HPP_
#define INCLUDE_CORE_STATES_HPP_

#include <memory>
#include <mutex>
#include <functional>
#include <chrono>
#include <vector>

#include "core/config.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/constants.hpp"

class Tick;

class MissionState {
    public:
virtual ~MissionState() = default;
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
~PreparationState() override = default;
        MissionState* tick() override;

        std::string getName() override {
            return "Mission Preparation";
        }

    private:
        std::optional<Polygon> flightBoundary;
        std::optional<Polygon> airdropBoundary;
        std::optional<Polyline> waypoints;
        std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES> bottles;
};

#endif  // INCLUDE_CORE_STATES_HPP_
