#ifndef CAMERA_LUCID_HPP_
#define CAMERA_LUCID_HPP_

#ifdef ARENA_SDK_INSTALLED

#include "interface.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <optional>
using json = nlohmann::json;

void LucidCameraConfig::setConfig(json config) 
{
    this->Key_AcquisitionFrameRateEnable = CameraConfigMetadata<bool>("Key_AcquisitionFrameRateEnable", config["Key_AcquisitionFrameRateEnable"], true, false);
    this->Key_AcquisitionFrameRate = CameraConfigMetadata<float>("Key_AcquisitionFrameRate", config["Key_AcquisitionFrameRate"], true, false);
    this->Key_AcquisitionMode = CameraConfigMetadata<std::string>("Key_AcquisitionMode", config["Key_AcquisitionMode"], true, false);

    this->Key_ExposureAuto = CameraConfigMetadata<std::string>("Key_ExposureAuto", config["Key_ExposureAuto"], true, false);
    if (this->Key_ExposureAuto.getValue() == "Off")
    {
        this->Key_ExposureTime = CameraConfigMetadata<float>("Key_ExposureTime", config["Key_ExposureTime"], true, false);
    }
    else
    {
        std::cout << "Expouser set to Auto, skipping expouse time"
    }

    this->Key_GainAuto = CameraConfigMetadata<std::string>("Key_GainAuto", config["Key_GainAuto"], true, false);
    if (this->Key_GainAuto.getValue() == "Off")
    {
        this->Key_Gain = CameraConfigMetadata<float>("Key_Gain", config["Key_Gain"], true, false);
    }
    else
    {
        std::cout << "Gain set to Auto, skipping."
    }
    this->Key_TriggerSource = CameraConfigMetadata<std::string>("Key_TriggerSource", config["Key_TriggerSource"], true, false);
    this->Key_TriggerMode = CameraConfigMetadata<std::string>("Key_TriggerMode", config["Key_TriggerMode"], true, false);
    this->Key_TriggerArmed = CameraConfigMetadata<float>("Key_TriggerArmed", config["Key_TriggerArmed"], false, false);
    this->Key_TriggerSelector = CameraConfigMetadata<std::string>("Key_TriggerSelector", config["Key_TriggerSelector"], true, false);
    // idk what to do with this
    // this->Key_TriggerSoftware = CameraConfigMetadata<bool>("Key_TriggerSoftware", config["Key_TriggerSoftware"], false, true);
    this->Key_UserSetSelector = CameraConfigMetadata<std::string>("Key_UserSetSelector", config["Key_UserSetSelector"], true, false);
    // idk what to do with this
    // this->Key_UserSetLoad = CameraConfigMetadata<bool>("Key_UserSetLoad", config["Key_UserSetLoad"], false, true);
    this->Key_PixelFormat = CameraConfigMetadata<std::string>("Key_PixelFormat", config["Key_PixelFormat"], true, false);
    this->Key_GevPersistentARPConflictDetectionEnable = CameraConfigMetadata<bool>("Key_GevPersistentARPConflictDetectionEnable", config["Key_GevPersistentARPConflictDetectionEnable"], true, false);
    this->Key_BalanceWhiteEnable = CameraConfigMetadata<bool>("Key_BalanceWhiteEnable", config["Key_BalanceWhiteEnable"], true, false);
    this->Key_Gamma = CameraConfigMetadata<float>("Key_Gamma", config["Key_Gamma"], true, false);
    this->Key_BlackLevel = CameraConfigMetadata<float>("Key_BlackLevel", config["Key_BlackLevel"], true, false);
    this->Key_BalanceWhiteAuto = CameraConfigMetadata<std::string>("Key_BalanceWhiteAuto", config["Key_BalanceWhiteAuto"], true, false);
}

LucidCameraConfig::LucidCameraConfig(json config) {
    this->configJson = config;
    setConfig(this->configJson);
}

void LucidCameraConfig::updateConfig(json newSetting)
{
    this->configJson = newSetting;
    setConfig(this->configJson);
}


// TODO: skipped for now. This is purely for testing
void LucidCameraConfig::updateConfigField(std::string key, T value) 
{

    switch (key)
    {
    case "Key_AcquisitionFrameRateEnable":
        this->Key_AcquisitionFrameRateEnable.setValue(value);
        break;

    case "Key_AcquisitionFrameRate":
        this->Key_AcquisitionFrameRate.setValue(value);
        break;

    case "Key_AcquisitionMode":
        this->Key_AcquisitionMode.setValue(value);
        break;

    case "Key_ExposureTime" :
        if (this->Key_ExposureAuto.getValue() == "Off")
        {
            this->Key_ExposureTime.setValue(value);
        }
        else
        {
            std::cout << "Unable to update expouser time. Set to auto, skipping"
        }

        break;
    case "Key_Gain":
        if (this->Key_GainAuto.getValue() == "Off")
        {
            this->Key_Gain.setValue(value);
        }
        else
        {
            std::cout << "Unable to update key gain. Set to auto, skipping."
        }
        break;
    
    
    default : break;
    }

}

json LucidCameraConfig::getConfig() 
{
    return this->configJson;
}

json LucidCameraConfig::getConfigField(std::string name)
{
    json returnJ;
    if (this->configJson.contains(name))
    {
        returnJ[name] = this->configJson[name];
    } else {
        std::cout << "Field " + name + " doesn't exist" << std::endl;
    }

    return json;
}

LucidCamera::CameraInterface(CameraConfiguration * config)
{
    if (config != nullptr) {
        this->config = LucidCameraConfig(config); // huh?
    } else {
        this->config = nullptr;
    }
    this->uploadPath = uploadPath;
    this->system = Arena::OpenSystem();
    this->device = nullptr;
    this->doneTakingPicture = false;
    this->recentPicture = nullptr;

}

int LucidCamera::connect() 
{
    try
    {
        this->system->UpdateDevices(1000);
        std::vector<Arena::DeviceInfo> deviceInfos = this->system->GetDevices();
        if (deviceInfos.size() == 0)
        {
            std::cout << "\nNo camera connected\nPress enter to complete\n";
            std::getchar();
            return -1;
        }
        this->device = this->system(deviceInfos[0]);

        // run example
        std::cout << "Commence example\n\n";
        ConfigureTrigger();
        std::cout << "\nExample complete\n";

        // TODO: add to destructor
        // pSystem->DestroyDevice(pDevice);
        // Arena::CloseSystem(pSystem);
    }
    catch (GenICam::GenericException &ge)
    {
        std::cout << "\nGenICam exception thrown: " << ge.what() << "\n";
        return -1;
    }
    catch (std::exception &ex)
    {
        std::cout << "Standard exception thrown: " << ex.what() << "\n";
        return -1;
    }
    catch (...)
    {
        std::cout << "Unexpected exception thrown\n";
        return -1;
    }
}

void LucidCamera::configureTrigger() 
{
    return 0;
}

void LucidCamera::verifyConnection()
{
    return 0;
}

ImageData takePicture() 
{
    return nullptr;
}

ImageData getLastPicture()
{
    return nullptr;
}

#endif // ARENA_SDK_INSTALLED

#endif // CAMERA_LUCID_HPP_