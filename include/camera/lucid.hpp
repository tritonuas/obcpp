#ifndef CAMERA_LUCID_HPP_
#define CAMERA_LUCID_HPP_
#define ARENA_SDK_INSTALLED
#ifdef ARENA_SDK_INSTALLED

#include "camera/interface.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include "ArenaApi.h"
using json = nlohmann::json;

class LucidCameraConfig {
    private:
        json configJson;
    public:
        LucidCameraConfig();

        LucidCameraConfig(json config);

        void updateConfig(json newSetting);

        // void updateConfigField(std::string key, std::string value) override;

        // void updateConfigField(std::string key, int value) override;

        // void updateConfigField(std::string key, bool value) override;

        json getConfig();

        json getConfigField(std::string name);
};

class LucidCamera {
    private:
        LucidCameraConfig * config;
        ImageData* recentPicture; // might need to move it to public
        Arena::IDevice * device;
        Arena::ISystem * system;
        ImageData * imgConvert(Arena::IImage * pImage); 
        void configureTrigger();

    public:
        LucidCamera(LucidCameraConfig * config);

        int connect();

        bool verifyConnection();

        ImageData *  takePicture(int timeout);

        ImageData * getLastPicture();

        // bool takePictureForSeconds(int sec) override;

        // void startTakingPictures(double intervalSec) override;

        // bool isDoneTakingPictures() override;

        // CameraConfiguration getConfig() override;

        // void updateConfig(CameraConfiguration newConfig) override;

        // void updateConfig(json newJsonConfig) override;

};

#endif // ARENA_SDK_INSTALLED

#endif // CAMERA_LUCID_HPP_
