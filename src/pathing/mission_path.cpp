#include "pathing/mission_path.hpp"

#include <vector>

#include "protos/obc.pb.h"
#include "utilities/logging.hpp"

MissionPath::MissionPath(MissionPath::Type type, std::vector<GPSCoord> path, int hover_wait_s):
    type(type), path(path), hover_wait_s(hover_wait_s)
{
    switch (type) {
        case Type::HOVER:
            generateHoverCommands();
            break;
        default:
            LOG_F(WARNING, "Unknown MissionPath type %d, defaulting to forward", static_cast<int>(type));
        case Type::FORWARD:
            generateForwardCommands();
            break;
    }
}

const std::vector<GPSCoord>& MissionPath::get() const {
    return this->path;
}

const std::vector<mavsdk::MissionRaw::MissionItem> MissionPath::getCommands() const {
    return this->path_mav;
}

void MissionPath::generateForwardCommands() {
    this->path_mav.clear();  // incase this gets called multiple times, but it shouldnt

    // Parse the waypoint information
    int i = 0;
    for (const auto& coord : this->path) {
        mavsdk::MissionRaw::MissionItem item{};
        item.seq = i;
        item.frame = MAV_FRAME_GLOBAL_RELATIVE_ALT;
        item.command = MAV_CMD_NAV_WAYPOINT;
        item.current = (i == 0) ? 1 : 0;
        item.autocontinue = 1;
        item.param1 = 0.0;  // Hold
        item.param2 = 7.0;  // Accept Radius 7.0m close to 25ft
        item.param3 = 0.0;  // Pass Radius
        item.param4 = NAN;  // Yaw
        item.x = int32_t(std::round(coord.latitude() * 1e7));
        item.y = int32_t(std::round(coord.longitude() * 1e7));
        item.z = coord.altitude();
        item.mission_type = MAV_MISSION_TYPE_MISSION;
        this->path_mav.push_back(item);
        i++;
    }
}

void MissionPath::generateHoverCommands() {
    this->path_mav.clear();

    // https://mavlink.io/en/messages/common.html#MAV_CMD_NAV_LOITER_TIME
    int i = 0;
    for (const auto& coord : this->path) {
        mavsdk::MissionRaw::MissionItem item{};
        item.seq = i;
        item.frame = MAV_FRAME_GLOBAL_RELATIVE_ALT;
        item.command = MAV_CMD_NAV_LOITER_TIME;
        item.current = (i == 0) ? 1 : 0;
        item.autocontinue = 1;
        item.param1 = static_cast<float>(this->hover_wait_s);
        item.param2 = 0.0f;  // 0 => dont need to point heading at next waypoint
        item.param3 = 0.0f;  // loiter radius, which shouldn't matter for hover quadplane
        item.param4 = 0.0f;  // xtrack, something for forward flight planes which we arent here
        item.x = int32_t(std::round(coord.latitude() * 1e7));
        item.y = int32_t(std::round(coord.longitude() * 1e7));
        item.z = coord.altitude();
        item.mission_type = MAV_MISSION_TYPE_MISSION;
        this->path_mav.push_back(item);
        i++;
    }
}
