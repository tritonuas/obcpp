/* ObstacleEvasionState*/

ObstacleEvasionState::ObstacleEvasionState(std::vector<GPSCoord> obstacles) {
    ObstacleEvasionState::obstacles = obstacles;
}

MissionState* ObstacleEvasionState::tick() {
    // TODO: Return state change when done evading
    // 
    return nullptr;
}

MissionState* ObstacleEvasionState::evade() {
    // There are 3 good options for doing avoidance. 
    // - Overwrite the next waypoint
    // - Continue along path but edit next altitude (immediately start climbing/descending)
    // - Enter a loiter (go in circles if fixed wing/enter VTOL and hold poistion) and climb/descend
    // https://mavlink.io/en/messages/common.html#MAV_CMD_NAV_SPLINE_WAYPOINT
    // https://mavlink.io/en/messages/common.html#MAV_CMD_NAV_CONTINUE_AND_CHANGE_ALT
    // https://mavlink.io/en/messages/common.html#MAV_CMD_NAV_LOITER_TO_ALT

    
    return nullptr;
}

void ObstacleEvasionState::updateObstaclesList(std::vector<GPSCoord> obstacles) {
    // TODO:
    return 0;
}
