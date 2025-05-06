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

void saveImageTelemetryToFile(const ImageTelemetry& telemetry,
                              const std::filesystem::path& filepath) {
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
    // std::cout << "Failed to save telemetry json to " << filepath.string().c_str() << '\n';
    return;
  }
  telemetry_file << to_string(telemetry_json);
}

bool ImageData::saveToFile(std::string directory) const {
    try {
        std::filesystem::path save_dir = directory;
        std::filesystem::path img_filepath =
            save_dir / (std::to_string(this->TIMESTAMP) + std::string(".jpg"));
        std::filesystem::path json_filepath =
            save_dir / (std::to_string(this->TIMESTAMP) + std::string(".json"));
        saveImageToFile(this->DATA, img_filepath);
        if (this->TELEMETRY.has_value()) {
            saveImageTelemetryToFile(this->TELEMETRY.value(), json_filepath);
        }
    } catch (std::exception& e) {
        LOG_F(ERROR, "Failed to save image and telemetry to file");
        // std::cout << "Failed to save image and telemetry to file" << '\n';
        return false;
    }

    return true;
}
