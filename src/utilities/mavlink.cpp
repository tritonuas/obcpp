#include "utilities/mavlink.hpp"

// to manually send a command which is not implemented by mavsdk
// You need to use `mavlink_passthrough` which has has a few send_command functions
int sendCustomMavlinkCommand(
    uint8_t target_sysid, uint8_t target_compid, uint16_t command, float param1, float param2, float param3, float param4, float param5, float param6, float param7, 
) {
    // https://mavsdk.mavlink.io/main/en/cpp/api_reference/structmavsdk_1_1_mavlink_passthrough_1_1_command_long.html

    // These two can probably be gotten from the mavlink object, rather than requiring parameter
    // uint8_t target_sysid = 0;
    // uint8_t target_compid = 0;

    mavsdk::MavlinkPassthrough::CommandLong command = mavsdk::MavlinkPassthrough::CommandLong(
    target_sysid, target_compid, command, param1, param2, param3, param4, param5, param6, param7, 
    );
    // Result mavsdk::MavlinkPassthrough::send_command_long(const CommandLong &command)
    mavsdk::MavlinkPassthrough::send_command_long(command);
}
