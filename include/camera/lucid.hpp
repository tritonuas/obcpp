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

using namespace std::chrono_literals;

class LucidCameraConfig {
    private:
        json configJson;
    public:
        LucidCameraConfig();

        LucidCameraConfig(json config);
        void updateConfig(json newSetting);

        json getConfig();

        json getConfigField(std::string name);

        // void updateConfigField(std::string key, std::string value) override;

        // void updateConfigField(std::string key, int value) override;

        // void updateConfigField(std::string key, bool value) override;
};

class LucidCamera : public CameraInterface {
 public:
    explicit LucidCamera(CameraConfiguration config);
    ~LucidCamera() = default;

    void connect() override;
    bool isConnected() override;

    void startTakingPictures(std::chrono::seconds interval) override;
    void stopTakingPictures() override;

    std::optional<ImageData> getLatestImage() override;
    std::queue<ImageData> getAllImages() override;


 private:
   ImageData imgConvert(Arena::IImage * pImage); 
   ImageData takePicture(int timeout);
   void captureEvery(std::chrono::seconds interval);
   void configureDefaults();

   LucidCameraConfig* config;

   inline static std::shared_mutex arenaSystemLock;
   inline static Arena::ISystem* system;

   inline static std::shared_mutex arenaDeviceLock;
   inline static Arena::IDevice* device;

   std::atomic_bool isTakingPictures;

   std::queue<ImageData> imageQueue;
   std::shared_mutex imageQueueLock;

   std::thread captureThread;

   std::shared_mutex imageQueueMut;

   const int connectionTimeoutMs = 1000;
   const std::chrono::milliseconds connectionRetry = 1000s;

   const int takePictureTimeoutSec = 1;
};


// #endif  // ARENA_SDK_INSTALLED

#endif  // INCLUDE_CAMERA_LUCID_HPP_
