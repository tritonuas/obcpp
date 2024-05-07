#include "camera/interface.hpp"

#include <optional>

CameraInterface::CameraInterface(const CameraConfig& config) : config(config) {}

std::optional<ImageTelemetry> queryMavlinkImageTelemetry(std::shared_ptr<MavlinkClient> mavlinkClient) {
  if (mavlinkClient == nullptr) {
    return {};
  }

  auto [lat_deg, lon_deg] = mavlinkClient->latlng_deg();
  double altitude_agl_m = mavlinkClient->altitude_agl_m();
  double airspeed_m_s = mavlinkClient->airspeed_m_s();
  double heading_deg = mavlinkClient->heading_deg();
  double yaw_deg = mavlinkClient->yaw_deg();
  double pitch_deg = mavlinkClient->pitch_deg();
  double roll_deg = mavlinkClient->roll_deg();

  return ImageTelemetry {
    .latitude_deg = lat_deg,
    .longitude_deg = lon_deg,
    .altitude_agl_m = altitude_agl_m,
    .airspeed_m_s = airspeed_m_s,
    .heading_deg = heading_deg,
    .yaw_deg = yaw_deg,
    .pitch_deg = pitch_deg,
    .roll_deg = roll_deg
  };
}