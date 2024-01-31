#ifndef INCLUDE_UTILITIES_MAVLINK_HPP_
#define INCLUDE_UTILITIES_MAVLINK_HPP_

#include <mavsdk/mavsdk.h>

#include <cstdint>

/**
 * Send a custom Mavlink command which is not implemented by mavsdk.
 * You have to check the mavlink documentation and supply all the arguments yourself.
 * WARNING: There are no compile time checks that you are passing in the right data!
 *          Calls to this MUST be carefully checked!
 */
int sendCustomMavlinkCommand(uint8_t target_sysid, uint8_t target_compid, uint16_t command,
                             float param1, float param2, float param3, float param4, float param5,
                             float param6, float param7, );

#endif  // INCLUDE_UTILITIES_MAVLINK_HPP_
