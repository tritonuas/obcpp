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
        CameraConfigMetadata<bool> Key_AcquisitionFrameRateEnable;
        CameraConfigMetadata<float> Key_AcquisitionFrameRate;
        CameraConfigMetadata<std::string> Key_AcquisitionMode;
        CameraConfigMetadata<float> Key_ExposureTime;
        CameraConfigMetadata<std::string> Key_ExposureAuto;
        CameraConfigMetadata<std::string> Key_GainAuto;
        CameraConfigMetadata<float> Key_Gain;
        CameraConfigMetadata<std::string> Key_TriggerSource;
        CameraConfigMetadata<std::string> Key_TriggerMode;
        CameraConfigMetadata<bool> Key_TriggerArmed;
        CameraConfigMetadata<std::string> Key_TriggerSelector;
        CameraConfigMetadata<bool> Key_TriggerSoftware; // ! OK: this is actually a None and its either in config or not so we are using bool here rn
        CameraConfigMetadata<std::string> Key_UserSetSelector;
        CameraConfigMetadata<bool> Key_UserSetLoad; // ! same here
        CameraConfigMetadata<std::string> Key_PixelFormat;
        CameraConfigMetadata<bool> Key_GevPersistentARPConflictDetectionEnable;
        // "not exactly sure what the following types should be ..." - from old obc repo
        CameraConfigMetadata<bool> Key_BalanceWhiteEnable;
        CameraConfigMetadata<bool> Key_GammaEnable;
        CameraConfigMetadata<float> Key_Gamma;
        CameraConfigMetadata <float> Key_BlackLevel;
        CameraConfigMetadata<std::string> Key_BalanceWhiteAuto;

        void setConfig(json config);
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
        ImageData imgConvert(Arena::IImage * pImage); 
        void configureTrigger();

    public:
        LucidCamera(LucidCameraConfig * config);

        int connect();

        void verifyConnection();

        ImageData *  takePicture();

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
