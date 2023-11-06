#ifndef CAMERA_INTERFACE_HPP_
#define CAMERA_INTERFACE_HPP_
#include <nlohmann/json.hpp>
#include <string>
// TODO: import OpenCV library here
template <typename T> 
class CameraConfigMetadata {
    private:
        const std::string name;
        T value;
        const std::string valueType;
        const bool configurable;
        const bool executable;
        const std::unordered_map<std::string, T> requirements;// key string name  - T value of dependency 
    public: 
        LucidCameraConfigMetadata(std::name, T value, bool configurable, bool executable, std::unordered_map<std::string, T> requirements) {
            this->name = name; 
            this->value = value;
            this->valueType = typeid(this->value).name();
            this->executable = executable;
            this->requirements = requirements;
        }

        std::string getName() {
            return this->name; 
        }

        T getValue() {
            return this->value;
        }

        void setValue(T value) {
            this->value = value; 
        }

        std::string getValueType() {
            return this->valueType;
        }

        std::unordered_map<std::string, T> getRequirments() {
            return this->requirements;
        }
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
class ImageData
{
private:
    const std::string NAME;
    const std::string PATH;
    const Mat DATA;

public:
    ImageData(std::string NAME, std::string PATH, Mat DATA);
    std::string getName();
    std::string getPath();
    Mat getData();
};

// ? possibly convert most common / important json fields to
// ? data fields
class CameraConfiguration
{
private:
    nlohmann::json configJson;

public:
    explicit CameraConfiguration(nlohmann::json config);

    void updateConfig(nlohmann::json newSetting);

    void updateConfigField(std::string key, T value);

    nlohmann::json getConfig();

    nlohmann::json getConfigField();
};

class CameraInterface
{
private:
    CameraConfiguration config;
        ImageData recentPicture; // might need to move it to public
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

#endif // CAMERA_INTERFACE_HPP_