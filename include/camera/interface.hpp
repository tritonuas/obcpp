#ifndef INCLUDE_CAMERA_INTERFACE_HPP_
#define INCLUDE_CAMERA_INTERFACE_HPP_

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>
#include <deque>
#include <filesystem>

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>

#include "network/mavlink.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/obc_config.hpp"

#include "libcamera/

using json = nlohmann::json;
using Mat = cv::Mat;

// struct to contain all telemetry that should be tagged with an image.
struct ImageTelemetry {
    double latitude_deg;
    double longitude_deg;
    double altitude_agl_m;
    double airspeed_m_s;
    double heading_deg;
    double yaw_deg;
    double pitch_deg;
    double roll_deg;
};

/**
 * Given a shared ptr to a mavlink client, query it for the telemetry
 * information needed for ImageTelemetry and the CV pipeline.
 * 
 * If nullptr is passed for mavlinkClient, nullopt is returned 
*/
std::optional<ImageTelemetry> queryMavlinkImageTelemetry(
    std::shared_ptr<MavlinkClient> mavlinkClient);

struct ImageData {
    cv::Mat DATA;
    uint64_t TIMESTAMP;
    std::optional<ImageTelemetry> TELEMETRY;

    /**
     * Saves the image to a file
     * @param directory directory to save the image in
     * @returns true/false if saving was successful
     */
    bool saveToFile(std::string directory) const;
};

std::string cvMatToBase64(cv::Mat image);

void saveImageToFile(cv::Mat image, const std::filesystem::path& filepath);

void saveImageTelemetryToFile(const ImageTelemetry& telemetry,
                              const std::filesystem::path& filepath);

class CameraInterface {
 protected:
    CameraConfig config;

 public:
    explicit CameraInterface(const CameraConfig& config) {
        //Make and start the camera manager
        std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
        cm->start();

        //Get the camera
        if (cm->cameras().empty()) {
            std::cout << "No cameras were identified on the system."
                << std::endl;
            cm->stop();
            return;
        }

                std::unique_ptr<CameraConfiguration> config =
            camera->generateConfiguration( { StreamRole::Raw } );

        /*
         * Config the stream
         */
        StreamConfiguration &streamConfig = config->at(0);
        streamConfig.size = { 2028, 1520 };
        std::cout << "Default Raw configuration is: "
            << streamConfig.toString() << std::endl;
        streamConfig.bufferCount = BUFFER_COUNT;

        streamConfig.pixelFormat = streamConfig.pixelFormat.fromString("YUV420");

        std::cout << "pixelFormat" << streamConfig.pixelFormat.toString();

                /*
         * Validating a CameraConfiguration -before- applying it will adjust it
         * to a valid configuration which is as close as possible to the one
         * requested.
         */
        config->validate();
        std::cout << "Validated viewfinder configuration is: "
            << streamConfig.toString() << std::endl;

        /*
         * Once we have a validated configuration, we can apply it to the
         * Camera.
         */
        camera->configure(config.get());

        //Allocate the buffers
        FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);
        for (StreamConfiguration &cfg : *config) {
            int ret = allocator->allocate(cfg.stream());
            if (ret < 0) {
                std::cerr << "Can't allocate buffers" << std::endl;
                return;
                //return EXIT_FAILURE;
            }

            size_t allocated = allocator->buffers(cfg.stream()).size();
            std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;
        }

        //Make the requests
        Stream *stream = streamConfig.stream();
        const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
        for (unsigned int i = 0; i < buffers.size(); ++i) {
            std::unique_ptr<Request> request = camera->createRequest();
            if (!request)
            {
                std::cerr << "Can't create request" << std::endl;
                return;

            }

            const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
            int ret = request->addBuffer(stream, buffer.get());
            if (ret < 0)
            {
                std::cerr << "Can't set buffer for request"
                    << std::endl;
                return;
            }

            /*
             * Controls can be added to a request on a per frame basis.
             */
            //ControlList &controls = request->controls();
            //controls.set(controls::AnalogueGain, 0.5);

            requests.push_back(std::move(request));
        }

        //Make the camera call this function every time a request is complete
        camera->requestCompleted.connect(requestComplete);

                /*
         * --------------------------------------------------------------------
         * Start Capture
         *
         * For each delivered frame, the Slot connected to the
         * Camera::requestCompleted Signal is called.
         */
        camera->start();
        for (std::unique_ptr<Request> &request : requests) {

            std::cout << std::endl
                << "Added Request: " << request->toString() << std::endl;
            camera->queueRequest(request.get());
        }

        /*
         * Run an EventLoop
         */
        loop.timeout(TIMEOUT_SEC);
        //int ret = loop.exec();
        std::thread cameraThread(&EventLoop::exec, &loop);
        std::vector<std::unique_ptr<Request>> requests;
        //cameraThread.detach();
        cameraThread.join();

        /*
         * Clean Up
         */
        camera->stop();
        allocator->free(stream);
        delete allocator;
        camera->release();
        camera.reset();
        cm->stop();

    }





    }
    virtual ~CameraInterface() = default;

    virtual void connect() = 0;
    virtual bool isConnected() = 0;

    /**
     * Start taking photos at an interval in a background thread.
     * Also requires a shared_ptr to a MavlinkClient to tag 
     * images with flight telemetry at capture time.
    */
    virtual void startTakingPictures(const std::chrono::milliseconds& interval,
    std::shared_ptr<MavlinkClient> mavlinkClient) = 0;
    /**
     * Close background thread started by startTakingPictures
    */
    virtual void stopTakingPictures() = 0;

    // Get the latest buffered image
    virtual std::optional<ImageData> getLatestImage() = 0;
    // Get all the recently buffered images
    virtual std::deque<ImageData> getAllImages() = 0;

    virtual void startStreaming() = 0;

    /**
    * Blocking call that takes an image. If it takes longer than the timeout 
    * to capture the image, no image is returned.
    */
    virtual std::optional<ImageData> takePicture(const std::chrono::milliseconds& timeout,
        std::shared_ptr<MavlinkClient> mavlinkClient) = 0;
};

#endif  // INCLUDE_CAMERA_INTERFACE_HPP_
