
// to manually send a command which is not implemented by mavsdk
// You need to use `mavlink_passthrough` which has has a few send_command functions
int sendCustomMavlinkCommand(
    target_sysid, target_compid, command, param1, param2, param3, param4, param5, param6, param7, 
) {
    // https://mavsdk.mavlink.io/main/en/cpp/api_reference/structmavsdk_1_1_mavlink_passthrough_1_1_command_long.html

    // These two can probably be gotten from the mavlink object
    uint8_t target_sysid = 0;
    uint8_t target_compid = 0;

    // Mavlink command number
    uint16_t command = 30;

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

    mavsdk::MavlinkPassthrough::CommandLong command = mavsdk::MavlinkPassthrough::CommandLong(
    target_sysid, target_compid, command, param1, param2, param3, param4, param5, param6, param7, 
    );
    // Result mavsdk::MavlinkPassthrough::send_command_long(const CommandLong &command)
    mavsdk::MavlinkPassthrough::send_command_long(command);
}
