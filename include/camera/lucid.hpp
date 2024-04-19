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

};

// #endif  // ARENA_SDK_INSTALLED

#endif  // INCLUDE_CAMERA_LUCID_HPP_
