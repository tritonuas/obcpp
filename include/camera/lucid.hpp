<<<<<<< HEAD
#ifndef INCLUDE_CAMERA_LUCID_HPP_
#define INCLUDE_CAMERA_LUCID_HPP_

// #ifdef ARENA_SDK_INSTALLED

#include <memory>

#include "camera/interface.hpp"

class LucidCamera : public CameraInterface {
 public:
    LucidCamera();
    ~LucidCamera();

    void connect();
    bool isConnected();

    void startTakingPictures(std::chrono::seconds interval) override;
    void stopTakingPictures() override;

    std::optional<ImageData> getLatestImage() override;
    std::queue<ImageData> getAllImages() override;

 private:
   static std::shared_mutex arenaSystemLock;
   static Arena::ISystem* pSystem;


   std::atomic_bool isConnected;

   std::atomic_bool isTakingPictures;

   void captureEvery(std::chrono::seconds interval);

   std::queue<ImageData> imageQueue;
   std::shared_mutex imageQueueLock;

   std::thread captureThread;

   ImageData takePicture();
    std::shared_mutex imageQueueMut;

=======
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

>>>>>>> origin/feat/lucid-config-interface
};

// #endif  // ARENA_SDK_INSTALLED

#endif  // INCLUDE_CAMERA_LUCID_HPP_
