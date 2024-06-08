#include "camera/interface.hpp"

#include <optional>
#include <filesystem>
#include <ostream>

#include "nlohmann/json.hpp"
#include <loguru.hpp>

#include "utilities/base64.hpp"

using json = nlohmann::json;

CameraInterface::CameraInterface(const CameraConfig& config) : config(config) {}

std::string cvMatToBase64(cv::Mat image) {
    std::vector<uchar> buf;
    cv::imencode(".jpg", image, buf);
    auto *enc_msg = reinterpret_cast<unsigned char*>(buf.data());
    return base64_encode(enc_msg, buf.size());
}

void saveImageToFile(cv::Mat image, const std::filesystem::path& filepath) {
  cv::imwrite(filepath, image);
}

void saveImageTelemetryToFile(const ImageTelemetry& telemetry, const std::filesystem::path& filepath) {  // NOLINT
  json telemetry_json = {
    {"latitude_deg", telemetry.latitude_deg },
    {"longitude_deg", telemetry.longitude_deg },
    {"altitude_agl_m", telemetry.altitude_agl_m },
    {"airspeed_m_s", telemetry.airspeed_m_s },
    {"heading_deg", telemetry.heading_deg },
    {"yaw_deg", telemetry.yaw_deg },
    {"pitch_deg", telemetry.pitch_deg },
    {"roll_deg", telemetry.roll_deg }
  };
  std::ofstream telemetry_file(filepath);
  if (!telemetry_file.is_open()) {
    LOG_F(ERROR, "Failed to save telemetry json to %s", filepath.string().c_str());
    return;
  }
  telemetry_file << to_string(telemetry_json);
}

std::optional<ImageTelemetry> queryMavlinkImageTelemetry(
  std::shared_ptr<MavlinkClient> mavlinkClient) {
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
