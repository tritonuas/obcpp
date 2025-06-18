#ifndef INCLUDE_PICAMERA_HPP_
#define INCLUDE_PICAMERA_HPP_

#include <string>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <deque>

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

#include "camera/interface.hpp"
#include "network/mavlink.hpp"

class PiCamera : public CameraInterface {
    public: 
        explicit PiCamera(CameraConfig config);
        ~PiCamera();

        void connect() override;
        bool isConnected() override;

        /**
         * Start taking photos at an interval in a background thread.
         * Also requires a shared_ptr to a MavlinkClient to tag 
         * images with flight telemetry at capture time.
        */
        void startTakingPictures(const std::chrono::milliseconds& interval,
            std::shared_ptr<MavlinkClient> mavlinkClient) override;

        /**
         * Close background thread started by startTakingPictures
        */
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
        /**
        * Takes an image and sleeps for the specified interval before
        * taking another image
        */
        void captureEvery(const std::chrono::milliseconds& interval,
            std::shared_ptr<MavlinkClient> mavlinkClient);   

        std::atomic_bool isTakingPictures;

	cv::VideoCapture cap;
            
        std::deque<ImageData> imageQueue;
        std::shared_mutex imageQueueLock;

        std::thread captureThread;
};

#endif
