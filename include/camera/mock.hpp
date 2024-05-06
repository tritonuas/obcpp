#ifndef INCLUDE_CAMERA_MOCK_HPP_
#define INCLUDE_CAMERA_MOCK_HPP_

#include <thread>
#include <memory>
#include <shared_mutex>
#include <deque>

#include "camera/interface.hpp"
#include "utilities/datatypes.hpp"

class MockCamera : public CameraInterface {
 public:
    explicit MockCamera(CameraConfig config);
    ~MockCamera();

    void connect() override;
    bool isConnected() override;

    void startTakingPictures(const std::chrono::milliseconds& interval) override;
    void stopTakingPictures() override;

    std::optional<ImageData> getLatestImage() override;
    std::deque<ImageData> getAllImages() override;

 private:
   std::vector<ImageData> mock_images;

   std::atomic_bool isTakingPictures;

   void captureEvery(const std::chrono::milliseconds& interval);

   std::deque<ImageData> imageQueue;
   std::shared_mutex imageQueueLock;

   std::thread captureThread;

   ImageData takePicture();
};

#endif  // INCLUDE_CAMERA_MOCK_HPP_
