/* ObstacleEvasionState*/

ObstacleEvasionState::ObstacleEvasionState(std::vector<GPSCoord> obstacles) {
    // TODO: Dear reviewer, I want this to take ownership of the obstacles vector.
    // How do I make that happen?
    ObstacleEvasionState::obstacles = obstacles;
}

MissionState* ObstacleEvasionState::tick() {
    // Return state change when done evading
    if (ObstacleEvasionState::evasionTimeRemaining <= 0) {
        return nullptr; // TODO: return the next state
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

    
    return nullptr;
}

void ObstacleEvasionState::updateObstaclesList(std::vector<GPSCoord> obstacles) {
    // TODO: Question for reviewers. Is this the best way to access class vars?
    // https://stackoverflow.com/questions/10198046/c-member-variables
    ObstacleEvasionState::obstacles = obstacles;
}
