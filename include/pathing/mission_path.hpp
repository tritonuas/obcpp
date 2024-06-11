#ifndef INCLUDE_PATHING_MISSION_PATH_HPP_
#define INCLUDE_PATHING_MISSION_PATH_HPP_

#include <vector>

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/mission_raw/mission_raw.h>

#include "utilities/datatypes.hpp"
#include "protos/obc.pb.h"

/**
 * Class which abstracts around a waypoint mission that we will upload via mavlink.
 * 
 * There are two kinds of mission paths: forward paths and hover paths
 * 
 * Forward paths will just be normal paths where you fly forward through all waypoints
 * 
 * Hover paths will be where you hover at each waypoint for a specified duration
 * 
 * It may make sense in the future to allow mixing these, so a mission path that contains
 * both hovering and forward flight, but at this time it makes more sense to keep it
 * simple with a binary choice on the entire path.
 */
class MissionPath {
 public:
    enum class Type {
        FORWARD, HOVER
    };

    MissionPath(Type type, std::vector<GPSCoord> path, int hover_wait_s = 5);
    MissionPath() = default;

    const std::vector<GPSCoord>& get() const;
    const std::vector<mavsdk::MissionRaw::MissionItem> getCommands() const;

 private:
    Type type;
    std::vector<GPSCoord> path;
    std::vector<mavsdk::MissionRaw::MissionItem> path_mav;
    int hover_wait_s;

    void generateHoverCommands();
    void generateForwardCommands();
};

#endif  // INCLUDE_PATHING_MISSION_PATH_HPP_
