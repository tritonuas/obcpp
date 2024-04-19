#ifndef INCLUDE_CAMERA_LUCID_HPP_
#define INCLUDE_CAMERA_LUCID_HPP_

// #ifdef ARENA_SDK_INSTALLED


#include "camera/interface.hpp"
#include <nlohmann/json.hpp>
#include "ArenaApi.h"

#include <string>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <thread>

#include "camera/interface.hpp"

using json = nlohmann::json;

class LucidCameraConfig {
    private:
        json configJson;
    public:
        LucidCameraConfig();

        LucidCameraConfig(json config);
        void updateConfig(json newSetting);

};

class LucidCamera : public CameraInterface {
 public:
    LucidCamera(LucidCameraConfig* config);
    ~LucidCamera();

    void connect();
    bool isConnected();

    void startTakingPictures(std::chrono::seconds interval) override;
    void stopTakingPictures() override;

    std::optional<ImageData> getLatestImage() override;
    std::queue<ImageData> getAllImages() override;

    void updateConfig(json newSetting);

    // void updateConfigField(std::string key, std::string value) override;

    // void updateConfigField(std::string key, int value) override;

    // void updateConfigField(std::string key, bool value) override;

    json getConfig();

    json getConfigField(std::string name);

 private:
   ImageData imgConvert(Arena::IImage * pImage); 
   ImageData takePicture(int timeout);
   void captureEvery(std::chrono::seconds interval);

   static std::shared_mutex arenaSystemLock;
   static Arena::ISystem* system;

   static std::shared_mutex arenaDeviceLock;
   static Arena::IDevice* device;

   std::atomic_bool isTakingPictures;


   std::queue<ImageData> imageQueue;
   std::shared_mutex imageQueueLock;

   std::thread captureThread;

   std::shared_mutex imageQueueMut;

   json configJson;

   const int connectionTimeoutMs = 1000;
   const int takePictureTimeoutSec = 1;
};


// #endif  // ARENA_SDK_INSTALLED

#endif  // INCLUDE_CAMERA_LUCID_HPP_
