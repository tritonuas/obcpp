#ifndef INCLUDE_CAMERA_MOCK_HPP_
#define INCLUDE_CAMERA_MOCK_HPP_

#include <thread>
#include <memory>
#include <shared_mutex>
#include <deque>
#include <vector>
#include <filesystem>

#include "camera/interface.hpp"
#include "network/mavlink.hpp"
#include "utilities/datatypes.hpp"

class MockCamera : public CameraInterface {
 public:
    explicit MockCamera(CameraConfig config);
    ~MockCamera();

    void connect() override;
    bool isConnected() override;

    /**
     * Start taking photos at an interval in a background thread
    */
    void startTakingPictures(const std::chrono::milliseconds& interval,
        std::shared_ptr<MavlinkClient> mavlinkClient) override;
    void stopTakingPictures() override;

    /**
     * Get the latest image that the camera took. This pops the latest
     * image from a queue of images which means that the same image won't.
     * be returned in two subsequent calls
    */ 
    std::optional<ImageData> getLatestImage() override;

    /**
     * getAllImages returns a queue of all the images that have been
     * cached since the last call to getAllImages. Once this is called,
     * it returns the cached images and clears the internal cache.
    */
    std::deque<ImageData> getAllImages() override;

    /**
    * Blocking call that takes an image. If it takes longer than the timeout 
    * to capture the image, no image is returned.
    */
    std::optional<ImageData> takePicture(const std::chrono::milliseconds& timeout,
        std::shared_ptr<MavlinkClient> mavlinkClient) override;

    void startStreaming() override;

 private:
    std::vector<ImageData> mock_images;

    std::atomic_bool isTakingPictures;

    void captureEvery(const std::chrono::milliseconds& interval,
        std::shared_ptr<MavlinkClient> mavlinkClient);

    std::deque<ImageData> imageQueue;
    std::shared_mutex imageQueueLock;

    std::thread captureThread;

    // Get telemetry from JSON file adjacent to given image file.
    // Ex: given path to "0003.jpg", telemetry will be looked for in 
    // "0003.json"
    std::optional<ImageTelemetry> getTelemetryFromJsonFile(std::filesystem::path img_path);
};

#endif  // INCLUDE_CAMERA_MOCK_HPP_
