#include "core/states.hpp"

#include <memory>
#include <mutex>

#include "core/config.hpp"
#include "core/ticks.hpp"
#include "utilities/locks.hpp"

// // in future might add to this
// MissionState::MissionState() = default;

// // Need to explicitly define now that Tick is no longer an incomplete class
// // See:
// // https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
// MissionState::~MissionState() = default;

// const MissionConfig& MissionState::getConfig() { return this->config; }

// std::chrono::milliseconds MissionState::doTick() {
//     Lock lock(this->tick_mut);

//     Tick* newTick = tick->tick();
//     if (newTick != nullptr) {
//         tick.reset(newTick);
//     }

//     return tick->getWait();
// }

// void MissionState::setTick(Tick* newTick) {
//     Lock lock(this->tick_mut);

//     tick.reset(newTick);
// }

// void MissionState::setInitPath(std::vector<GPSCoord> init_path) {
//     Lock lock(this->init_path_mut);
//     this->init_path = init_path;
// }

// const std::vector<GPSCoord>& MissionState::getInitPath() {
//     Lock lock(this->init_path_mut);
//     return this->init_path;
// }

// bool MissionState::isInitPathValidated() {
//     Lock lock(this->init_path_mut);
//     return this->init_path_validated;
// }

/* ObstacleEvasionState*/

ObstacleEvasionState::ObstacleEvasionState(std::vector<GPSCoord> obstacles) {
    ObstacleEvasionState::obstacles = obstacles;
}

MissionState* ObstacleEvasionState::tick() {
    // Return state change when done evading
    if (ObstacleEvasionState::evasionTimeRemaining <= 0) {
        return nullptr;  // TODO: return the next state
    } else {
        // decrement the time remaining to evade
        ObstacleEvasionState::evasionTimeRemaining -= 1;
    }

    return nullptr;
}

MissionState* ObstacleEvasionState::evade(void) {
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
    uint16_t command = 30;  // MAV_CMD_NAV_CONTINUE_AND_CHANGE_ALT

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

    int result = sendCustomMavlinkCommand(target_sysid, target_compid, command, param1, param2,
                                          param3, param4, param5, param6, param7);

    return nullptr;
}

void ObstacleEvasionState::updateObstaclesList(std::vector<GPSCoord> obstacles) {
    // TODO: Question for reviewers. Is this the best way to access class vars?
    // https://stackoverflow.com/questions/10198046/c-member-variables
    ObstacleEvasionState::obstacles = obstacles;
}
