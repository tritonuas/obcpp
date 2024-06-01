#ifndef INCLUDE_CORE_STATES_HPP_
#define INCLUDE_CORE_STATES_HPP_

#include <memory>
#include <mutex>
#include <functional>
#include <chrono>
#include <vector>

#include "core/mission_config.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/constants.hpp"
#include "ticks/ids.hpp"
#include "ticks/tick.hpp"
#include "protos/obc.pb.h"

/*
    State for when plane is actively evading a dynamic obstacle.
    Plane enters this state after a *dynamic* obstacle is detected. This state
    will choose a form of active evasion and then return control to normal
    pathing afterwards.
    The path planning does not run in real-time, so avoidance behaves by inserting
    emergency waypoints and then returning to continue the next unhit waypoint.
*/
class ObstacleEvasionState {
    public:
        // Do not use a default constructor. The evasion state needs to know
        // the location of the obstacle
        ObstacleEvasionState() = delete;
        // List of obstacles to avoid
        explicit ObstacleEvasionState(std::vector<GPSCoord> obstacles);
        ~ObstacleEvasionState() = default;

        TickID getTickID();
        std::chrono::milliseconds doTick();
        Tick* _tick();
        MissionConfig config;  // has its own mutex

        void updateObstaclesList(std::vector<GPSCoord> obstacles);

    private:
        std::mutex tick_mut;  // for reading/writing tick
        std::unique_ptr<Tick> tick;
        std::shared_ptr<MavlinkClient> mav;
        void _setTick(Tick* newTick);  // does not acquire the tick_mut

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
