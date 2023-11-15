#ifndef CORE_STATES_HPP_
#define CORE_STATES_HPP_

#include <string>
#include <array>
#include <optional>

#include "core/config.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/constants.hpp"

#include <memory>

/*
    Interface for an arbitrary mission state.
*/
class MissionState {
    public:
        MissionState(std::shared_ptr<MissionConfig> config);
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

    protected:
        std::shared_ptr<MissionConfig> config;
};

/*
    State for when the system has just been turned on and is waiting for
    mission parameters.
*/
class PreparationState: public MissionState {
    public:
        PreparationState(std::shared_ptr<MissionConfig> config);
        ~PreparationState() override = default;
        MissionState* tick() override;

        std::string getName() override {
            return "Mission Preparation";
        }
};

#endif // CORE_STATES_HPP_