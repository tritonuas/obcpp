// #define ARENA_SDK_INSTALLED
// #ifdef ARENA_SDK_INSTALLED

#include "camera/interface.hpp"
#include "camera/lucid.hpp"
#include <nlohmann/json.hpp>
#include "utilities/locks.hpp"

#include <chrono>
#include <thread>
#include <optional>
#include <string>
#include <unordered_map>

#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core.hpp>
#include <loguru.hpp>

using json = nlohmann::json;

LucidCameraConfig::LucidCameraConfig(json config) {
    this->configJson = config;
}

void LucidCameraConfig::updateConfig(json newSetting)
{
    this->configJson = newSetting;
}


// // TODO: skipped for now. This is purely for testing
// void LucidCameraConfig::updateConfigField(std::string key, std::string value)
// {
//     return;
// }

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

    return returnJ;
}

ImageData LucidCamera::imgConvert(Arena::IImage * pImage)
{
    Arena::IImage *pConverted = Arena::ImageFactory::Convert(
        pImage,
        BGR8);

    std::string name = "img_"+pConverted->GetTimestamp();
    void * data = (void *)pConverted->GetData();
    std::string path = "";

    cv::Mat mat = cv::Mat(static_cast<int>(pConverted->GetHeight()), static_cast<int>(pConverted->GetWidth()), CV_8UC3, data);
    cv::Mat matCopy = mat.clone();

    return ImageData(name, path, matCopy);
}

LucidCamera::LucidCamera(LucidCameraConfig *config)
{
    if (config != nullptr) {
        this->config = config; // huh?
    } else {
        this->config = nullptr;
    }
}


void LucidCamera::connect()
{
    try
    {
        WriteLock systemLock(this->arenaSystemLock);
        this->system->UpdateDevices(this->connectionTimeoutMs);

        std::vector<Arena::DeviceInfo> deviceInfos = this->system->GetDevices();
        if (deviceInfos.size() == 0)
        {
            LOG_F(ERROR, "\nNo camera connected\nPress enter to complete\n");
            std::getchar();
            throw std::exception();
        }
        WriteLock deviceLock(this->arenaDeviceLock);
        this->device = this->system->CreateDevice(deviceInfos[0]);

        // // run configuration
        // std::cout << "Trigger configuration start\n\n";
        // configureTrigger();
        // std::cout << "\nTrigger configured finished\n";

        if (!isConnected()) 
        {
            LOG_F(ERROR,"Lucid Camera Connection fail!\n");
            throw std::exception();
        }

        // TODO: add to destructor
        // pSystem->DestroyDevice(pDevice);
        // Arena::CloseSystem(pSystem);
    }
    catch (GenICam::GenericException &ge)
    {
        LOG_F(ERROR, "GenICam exception thrown: %s \n", ge.what());
        throw ge;
    }
    catch (std::exception &ex)
    {
        LOG_F(ERROR, "Standard exception thrown: %s \n", ge.what());
        throw ex;
    }
    catch (...)
    {
        LOG_F(ERROR, "Unexpected exception thrown: %s \n", ge.what());
        throw std::exception();
    }
}

void LucidCamera::startTakingPictures(std::chrono::seconds interval) {
    this->isTakingPictures = true;
    try {
        this->captureThread = std::thread(&LucidCamera::captureEvery, this, interval);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
void LucidCamera::stopTakingPictures() {
    if (!this->isTakingPictures) {
        return;
    }

    this->isTakingPictures = false;

    this->captureThread.join();
};

std::optional<ImageData> LucidCamera::getLatestImage() {
    ReadLock lock(this->imageQueueLock);
    ImageData lastImage = this->imageQueue.front();
    this->imageQueue.pop();
    return lastImage;
};

std::queue<ImageData> LucidCamera::getAllImages() {
    ReadLock lock(this->imageQueueLock);
    std::queue<ImageData> outputQueue = this->imageQueue; 
    this->imageQueue = std::queue<ImageData>();
    return outputQueue;
}

// void LucidCamera::configureTrigger() 
// {
//     json configJson = config->getConfig();

//     for (auto it = configJson.begin(); it != configJson.end(); it++)
//     {
//         std::string key = it.key();


//         if (it.value().type() == json::value_t::number_unsigned)
//         {
//             int value = it.value();
//             Arena::SetNodeValue<int64_t>(
//                 this->device->GetNodeMap(),
//                 key.c_str(),
//                 value);
//         }
//         else if (it.value().type() == json::value_t::boolean)
//         {
//             bool value = it.value();
//             Arena::SetNodeValue<bool>(
//                 this->device->GetNodeMap(),
//                 key.c_str(),
//                 value);
//         }
//         else if (it.value().type() == json::value_t::number_float)
//         {

//             float value_float = it.value();
//             double value = value_float;

//             Arena::SetNodeValue<double>(
//                 this->device->GetNodeMap(),
//                 key.c_str(),
//                 value);
//         }
//         else if (it.value().type() == json::value_t::string)
//         {
//             std::string value = it.value();
//             Arena::SetNodeValue<GenICam::gcstring>(
//                 this->device->GetNodeMap(),
//                 key.c_str(),
//                 value.c_str());
//         } else {
//             std::cout << "Unkown type of varible. Skipping " << key << std::endl;
//         }
//     }
// }

bool LucidCamera::isConnected() {
    ReadLock lock(this->arenaDeviceLock);
    return this->device->IsConnected();
}

void LucidCamera::captureEvery(std::chrono::seconds interval) {
    this->arenaDeviceLock.lock();
    this->device->StartStream();
    this->arenaDeviceLock.unlock();

    while (this->isTakingPictures) {
        LOG_F(INFO, "Taking picture with LUCID camera\n");
        ImageData newImage = this->takePicture(this->takePictureTimeoutSec);

        WriteLock lock(this->imageQueueLock);
        this->imageQueue.push(newImage);
        lock.unlock();

        std::this_thread::sleep_for(interval);
    }

    this->arenaDeviceLock.lock();
    this->device->StopStream();
    this->arenaDeviceLock.unlock();
}

ImageData LucidCamera::takePicture(int timeout) {
    WriteLock lock(this->arenaDeviceLock);
    Arena::IImage* pImage = device->GetImage(timeout);

    ImageData returnImg = imgConvert(pImage);

    if (pImage->IsIncomplete()) {
        LOG_F(ERROR, "Recivied Image is incomplete\n");
    }

    device->RequeueBuffer(pImage);
    device->StopStream();

    return returnImg;
}

// #endif // ARENA_SDK_INSTALLED
