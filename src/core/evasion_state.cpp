#include <memory>
#include <mutex>

#include "core/evasion_state.hpp"
#include "core/mission_config.hpp"
#include "utilities/locks.hpp"
#include "utilities/constants.hpp"
#include "core/mission_state.hpp"
#include "network/mavlink.hpp"
#include "ticks/mission_start.hpp"

ObstacleEvasionState::ObstacleEvasionState(std::vector<GPSCoord> obstacles) {
    ObstacleEvasionState::obstacles = obstacles;
}

std::chrono::milliseconds ObstacleEvasionState::doTick() {
    Lock lock(this->tick_mut);

    Tick* newTick = this->_tick(); // check for state change
    return DYNAMIC_AVOIDANCE_TICK_WAIT;  // run again in a second
}

Tick* ObstacleEvasionState::_tick() {
    // Return state change when done evading
    if (ObstacleEvasionState::evasionTimeRemaining <= 0) {
        // TODO: uncomment
        // return new MissionStartTick(ObstacleEvasionState::tick->state);
    }

    // Otherwise, decrement the time remaining to evade
    ObstacleEvasionState::evasionTimeRemaining -= 1;
    return nullptr;
}

TickID getTickID() {
    return TickID::EvasionState;
}

void ObstacleEvasionState::evade(void) {
    // Implementation details:
    // We have chosen to do option 2.
    // There are 3 good options for doing avoidance.
    // - Overwrite the next waypoint
    // - Continue along path but edit next altitude (immediately start climbing/descending)
    // - Enter a loiter (go in circles if fixed wing/enter VTOL and hold poistion) and climb/descend
    // https://mavlink.io/en/messages/common.html#MAV_CMD_NAV_SPLINE_WAYPOINT
    // https://mavlink.io/en/messages/common.html#MAV_CMD_NAV_CONTINUE_AND_CHANGE_ALT
    // https://mavlink.io/en/messages/common.html#MAV_CMD_NAV_LOITER_TO_ALT

    // These two can probably be gotten from the mavlink object
    uint8_t target_sysid = 0;
    uint8_t target_compid = 0;

    // Mavlink command number
    uint16_t command = MAV_CMD_NAV_CONTINUE_AND_CHANGE_ALT;

    // Climb or Descend (0 = Neutral, command completes when within 5m of this command's altitude,
    // 1 = Climbing, command completes when at or above this command's altitude, 2 = Descending,
    // command completes when at or below this command's altitude.
    float param1 = 1;

    // These are all unused.
    float param2 = 0.0f;
    float param3 = 0.0f;
    float param4 = 0.0f;
    float param5 = 0.0f;
    float param6 = 0.0f;
    float param7 = 0.0f;

    // TODO:(Samir) KMS, we need to compile mavlink to use plugins
    // TODO: type should be mavsdk::MavlinkPassthrough::Result
    mavsdk::Mission::Result result = ObstacleEvasionState::mav->sendCustomMavlinkCommand(target_sysid, target_compid, command, param1, param2,
                                          param3, param4, param5, param6, param7);
}

void ObstacleEvasionState::updateObstaclesList(std::vector<GPSCoord> obstacles) {
    // TODO: Question for reviewers. Is this the best way to access class vars?
    // https://stackoverflow.com/questions/10198046/c-member-variables
    ObstacleEvasionState::obstacles = obstacles;
}
