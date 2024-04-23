#ifndef INCLUDE_CAMERA_LUCID_HPP_
#define INCLUDE_CAMERA_LUCID_HPP_

#ifdef ARENA_SDK_INSTALLED


#include "camera/interface.hpp"
#include <nlohmann/json.hpp>
#include "ArenaApi.h"

#include <string>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <deque>

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
    ~LucidCamera();

    void connect() override;
    bool isConnected() override;

    void startTakingPictures(const std::chrono::milliseconds& interval) override;
    void stopTakingPictures() override;

    std::optional<ImageData> getLatestImage() override;
    std::deque<ImageData> getAllImages() override;


 private:
   std::optional<ImageData> takePicture(const std::chrono::milliseconds& timeout);
   void captureEvery(const std::chrono::milliseconds& interval);

   ImageData imgConvert(Arena::IImage * pImage); 
   void configureDefaults();

   LucidCameraConfig* config;

   inline static std::shared_mutex arenaSystemLock;
   inline static Arena::ISystem* system;

   inline static std::shared_mutex arenaDeviceLock;
   inline static Arena::IDevice* device;

   std::atomic_bool isTakingPictures;

   std::deque<ImageData> imageQueue;
   std::shared_mutex imageQueueLock;

   std::thread captureThread;

   std::shared_mutex imageQueueMut;

   const std::chrono::milliseconds connectionTimeout = 1000ms;
   const std::chrono::milliseconds connectionRetry = 500ms;

   const std::chrono::milliseconds takePictureTimeout = 1000ms;
};


#endif  // ARENA_SDK_INSTALLED

#endif  // INCLUDE_CAMERA_LUCID_HPP_
