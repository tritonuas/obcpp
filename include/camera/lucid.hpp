#ifndef CAMERA_LUCID_HPP_
#define CAMERA_LUCID_HPP_

#ifdef ARENA_SDK_INSTALLED

#include "interface.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>


class LucidCameraConfig: public CameraConfiguration {
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
        ConfigMetadata<std::string> Key_BalanceWhiteAuto
};

class LucidCamera : public CameraInterface {
    // override all the camera connection interface functions
};

#endif // ARENA_SDK_INSTALLED

#endif // CAMERA_LUCID_HPP_
