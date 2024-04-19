#ifndef INCLUDE_CAMERA_MOCK_HPP_
#define INCLUDE_CAMERA_MOCK_HPP_

#include <thread>
#include <memory>
#include <shared_mutex>

#include "camera/interface.hpp"

class MockCamera : public CameraInterface {
 public:
    explicit MockCamera(CameraConfiguration config);
    ~MockCamera();

    void connect() override;
    bool isConnected() override;

    void startTakingPictures(std::chrono::seconds interval) override;
    void stopTakingPictures() override;

    std::optional<ImageData> getLatestImage() override;
    std::queue<ImageData> getAllImages() override;

 private:
   std::atomic_bool isTakingPictures;

   void captureEvery(std::chrono::seconds interval);

   std::queue<ImageData> imageQueue;
   std::shared_mutex imageQueueLock;

   std::thread captureThread;

   ImageData takePicture();
};

#endif  // INCLUDE_CAMERA_MOCK_HPP_
