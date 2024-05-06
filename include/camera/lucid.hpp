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

/**
 * LucidCamera is the class to interface with cameras from 
 * LUCID Vision Labs (https://thinklucid.com/) that use their
 * Arena SDK (https://thinklucid.com/arena-software-development-kit/).
 * 
 * This class is thread safe, meaning that you can access camera resources
 * across different threads even though there's one physical camera.
*/
class LucidCamera : public CameraInterface {
 public:
    explicit LucidCamera(CameraConfig config);
    ~LucidCamera();

   /**
    * Connect to the LUCID camera. Note that this function will synchronously
    * block indefintely until a connection to the camera can be established.
   */
    void connect() override;
    bool isConnected() override;

    /**
     * Start taking photos at an interval in a background thread
    */
    void startTakingPictures(const std::chrono::milliseconds& interval) override;
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
 private:
   /**
    * Blocking call that takes an image. If it takes longer than the timeout 
    * to capture the image, no image is returned.
   */
   std::optional<ImageData> takePicture(const std::chrono::milliseconds& timeout);

   /**
    * Takes an image and sleeps for the specified interval before
    * taking another image
   */
   void captureEvery(const std::chrono::milliseconds& interval);

   /**
    * Converts between ArenaSDK Image type to an OpenCV cv::Mat 
   */
   ImageData imgConvert(Arena::IImage* pImage); 

   void configureDefaults();

   // Lock around Arena system.
   inline static std::shared_mutex arenaSystemLock;
   // Arena system must be static since only a single system
   // can be active on the same machine regardless of how 
   // many LucidCamera instances are created.
   inline static Arena::ISystem* system;

   // Lock around Arena device.
   inline static std::shared_mutex arenaDeviceLock;
   // Arena device must be static since only a single device 
   // can be active on the same machine regardless of how 
   // many LucidCamera instances are created.
   inline static Arena::IDevice* device;

   std::atomic_bool isTakingPictures;

   std::deque<ImageData> imageQueue;
   std::shared_mutex imageQueueLock;

   // thread that will capture images on a set interval
   std::thread captureThread;

   std::shared_mutex imageQueueMut;

   const std::chrono::milliseconds connectionTimeout = 1000ms;
   const std::chrono::milliseconds connectionRetry = 500ms;

   // TODO: need to catch timeout exception
   const std::chrono::milliseconds takePictureTimeout = 1000ms;
};


#endif  // ARENA_SDK_INSTALLED

#endif  // INCLUDE_CAMERA_LUCID_HPP_
