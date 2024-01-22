#ifndef CAMERA_INTERFACE_HPP_
#define CAMERA_INTERFACE_HPP_
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
using json = nlohmann::json;
using Mat = cv::Mat;

template <typename T> 
class CameraConfigMetadata {
    private:
        const std::string name;
        T value;
        const std::string valueType;
        const bool configurable;
        const bool executable;
    public: 
        CameraConfigMetadata(std::string name, T value, bool configurable, bool executable) {
            this->name = name; 
            this->value = value;
            this->valueType = typeid(this->value).name();
            this->executable = executable;
        }

        std::string getName()
        {
            return this->name;
        }

        T getValue()
        {
            return this->value;
        }

        void setValue(T value)
        {
            this->value = value;
        }

        std::string getValueType() 
        {
            return this->valueType;
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

    virtual void updateConfig(json newSetting) = 0;

    virtual void updateConfigField(std::string key, std::string value) = 0;

    virtual void updateConfigField(std::string key, int value) = 0;

    virtual void updateConfigField(std::string key, bool value) = 0;

    virtual json getConfig() = 0;

    virtual json getConfigField(std::string name) = 0;
};

class CameraInterface
{
private:
    CameraConfiguration *config;
    ImageData recentPicture; // might need to move it to public

public:
    explicit CameraInterface(CameraConfiguration config);

    virtual int connect() = 0;

    virtual void verifyConnection() = 0;

    virtual ImageData takePicture() = 0;

    virtual ImageData getLastPicture() = 0;
};

#endif // CAMERA_INTERFACE_HPP_