#ifndef INCLUDE_CAMERA_INTERFACE_HPP_
#define INCLUDE_CAMERA_INTERFACE_HPP_

#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

// class to contain all telemetry that should be tagged with an image.
// In the future this could be in a mavlink file.
class ImageTelemetry {
    public:
        ImageTelemetry(double latitude, double longitude, double altitude, 
            double airspeed, double yaw, double pitch, double roll);
        const double latitude;
        const double longitude;
        const double altitude;
        const double airspeed;
        const double yaw;
        const double pitch;
        const double roll;
};

/*
 * FYI: this is class that will standardize image data but
 * if all of our cameras have a uniform image output type
 * this class is completely and utterly useless.
 *                                      - Boris
 *
 * We will also need to develop a custom converter
 *                                      - Boris (10/11)
 */
class ImageData {
 private:
    const std::string NAME;
    const std::string PATH;
    const cv::Mat DATA;
    const ImageTelemetry TELEMETRY;

 public:
    ImageData(std::string NAME, std::string PATH, cv::Mat DATA, 
        ImageTelemetry TELEMETRY);
    std::string getName();
    std::string getPath();
    cv::Mat getData();
    ImageTelemetry getTelemetry();
};

// ? possibly convert most common / important json fields to
// ? data fields
class CameraConfiguration {
 private:
    nlohmann::json configJson;

 public:
    explicit CameraConfiguration(nlohmann::json config);

    void updateConfig(nlohmann::json newSetting);

    // void updateConfigField(std::string key, T value);

    nlohmann::json getConfig();

    nlohmann::json getConfigField();
};

class CameraInterface {
 private:
    CameraConfiguration config;
    ImageData recentPicture;  // might need to move it to public
    bool doneTakingPicture;      // overengineering time
    std::string uploadPath;
    // Interpreter interp
    // TODO: SERVER CONNECTION HERE ?

    void imgConvert();

 public:
    explicit CameraInterface(CameraConfiguration config);

    void connect();

    bool verifyConnection();

    void takePicture();

    ImageData getLastPicture();

    bool takePictureForSeconds(int sec);

    void startTakingPictures(double intervalSec);

    bool isDoneTakingPictures();

    CameraConfiguration getConfig();

    void updateConfig(CameraConfiguration newConfig);

    void updateConfig(nlohmann::json newJsonConfig);

    std::string getUploadPath();

    void setUploadPath(std::string path);

    void uploadPicture(ImageData img);

    std::vector<ImageData> listPicturesFromUploadPath();

    ImageData getImageByName(std::string name);

    // server connection methods here
    // virtual methods for all possible camera actions
};

#endif  // INCLUDE_CAMERA_INTERFACE_HPP_
