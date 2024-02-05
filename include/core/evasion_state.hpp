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
#include "protos/obc.pb.h"


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


/*
    State for when plane is actively evading a dynamic obstacle.
    Plane enters this state after a *dynamic* obstacle is detected. This state
    will choose a form of active evasion and then return control to normal
    pathing afterwards.
    The path planning does not run in real-time, so avoidance behaves by inserting
    emergency waypoints and then returning to continue the next unhit waypoint.
*/
class ObstacleEvasionState: public MissionState {
    public:
        // Do not use a default constructor. The evasion state needs to know
        // the location of the obstacle
        ObstacleEvasionState() override = delete;
        // List of obstacles to avoid
        explicit ObstacleEvasionState(std::vector<GPSCoord> obstacles);
        ~ObstacleEvasionState() override = default;

        MissionState* tick() override;

        std::string getName() override {
            return "Evading Dynamic Obstacle";
        }
        
        void updateObstaclesList(std::vector<GPSCoord> obstacles);

    private:
        // Evasion state will exit after not seeing obstacle for 15 seconds
        // or after it determines it has reached safety. Tick will decrement this
        // and exit state when this reaches 0.
        int evasionTimeRemaining;
        // List of obstacle locations.
        std::vector<GPSCoord> obstacles;
        /*
            Do the evasion logic and send mavlink commands.
        */
        void evade(void);
};


    // Wait until health is OK and vehicle is ready to arm
    // while (telemetry.health_all_ok() != true) {
        // std::cout << "Vehicle not ready to arm ..." << '\n';
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        // return nullptr;
    // }

    // return nullptr;


    
#endif  // INCLUDE_CORE_STATES_HPP_
