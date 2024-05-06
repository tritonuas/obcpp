#ifdef ARENA_SDK_INSTALLED

#include <chrono>
#include <thread>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core.hpp>
#include <loguru.hpp>

#include "camera/lucid.hpp"
#include "camera/interface.hpp"
#include "utilities/locks.hpp"
#include "utilities/datatypes.hpp"

using json = nlohmann::json;

LucidCamera::LucidCamera(CameraConfig config) :
    CameraInterface(config) {
}

void LucidCamera::connect() {
    if (this->isConnected()) {
        return;
    }

    // aquire locks to Arena System and Device
    WriteLock systemLock(this->arenaSystemLock);
    WriteLock deviceLock(this->arenaDeviceLock);

    CATCH_ARENA_EXCEPTION("opening Arena System",
        this->system = Arena::OpenSystem();
    );

    while (true) {
        CATCH_ARENA_EXCEPTION("attempting to connect to LUCID camera",
            // ArenaSystem broadcasts a discovery packet on the network to discover
            // any cameras on the network.
            // We provide a timeout in milliseconds for this broadcasting sequence.
            this->system->UpdateDevices(this->connectionTimeout.count());

            std::vector<Arena::DeviceInfo> deviceInfos = this->system->GetDevices();
            if (deviceInfos.size() != 0) {
                LOG_F(INFO,"Lucid camera connection succeeded!");
                this->device = this->system->CreateDevice(deviceInfos[0]);
                break;
            }
        );

        LOG_F(ERROR,"Lucid camera connection failed! Retrying in %ld ms", this->connectionRetry.count());
        std::this_thread::sleep_for(this->connectionRetry);
    }

    this->configureSettings();
}

LucidCamera::~LucidCamera() {
    // aquire locks to Arena System and Device
    WriteLock systemLock(this->arenaSystemLock);
    WriteLock deviceLock(this->arenaDeviceLock);

    CATCH_ARENA_EXCEPTION("closing Arena System",
        this->system->DestroyDevice(this->device);
        Arena::CloseSystem(this->system);
    );
}



void LucidCamera::startTakingPictures(const std::chrono::milliseconds& interval) {
    if (this->isTakingPictures) {
        return;
    }

    this->isTakingPictures = true;

    this->captureThread = std::thread(&LucidCamera::captureEvery, this, interval);
}

void LucidCamera::stopTakingPictures() {
    if (!this->isTakingPictures) {
        return;
    }

    this->isTakingPictures = false;

    this->captureThread.join();
};

void LucidCamera::configureSettings() {
    const std::string sensor_shutter_mode_name = "SensorShutterMode";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + sensor_shutter_mode_name).c_str(),
        Arena::SetNodeValue<GenICam::gcstring>(
            device->GetNodeMap(),
            sensor_shutter_mode_name.c_str(),
            this->config.lucid.sensor_shutter_mode.c_str());
    );

    const std::string acquisition_frame_rate_enable_name = "AcquisitionFrameRateEnable";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + acquisition_frame_rate_enable_name).c_str(),
        Arena::SetNodeValue<bool>(
            device->GetNodeMap(),
            acquisition_frame_rate_enable_name.c_str(),
            this->config.lucid.acquisition_frame_rate_enable);
    );

    // Note that this modifies the TLStreamNodeMap and not the standard NodeMap
    const std::string stream_auto_negotiate_packet_size_name = "StreamAutoNegotiatePacketSize";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + stream_auto_negotiate_packet_size_name).c_str(),
        Arena::SetNodeValue<bool>(
            device->GetTLStreamNodeMap(),
            stream_auto_negotiate_packet_size_name.c_str(),
            this->config.lucid.stream_auto_negotiate_packet_size);
    );

    // Note that this modifies the TLStreamNodeMap and not the standard NodeMap
    const std::string stream_packet_resend_enable_name = "StreamPacketResendEnable";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + stream_packet_resend_enable_name).c_str(),
        Arena::SetNodeValue<bool>(
            device->GetTLStreamNodeMap(),
            stream_packet_resend_enable_name.c_str(),
            this->config.lucid.stream_packet_resend_enable);
    );

    const std::string target_brightness_name = "TargetBrightness";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + target_brightness_name).c_str(),
        Arena::SetNodeValue<int64_t>(
            device->GetNodeMap(),
            target_brightness_name.c_str(),
            this->config.lucid.target_brightness);
    );

    const std::string gamma_enable_name = "GammaEnable";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + gamma_enable_name).c_str(),
        Arena::SetNodeValue<bool>(
            device->GetNodeMap(),
            gamma_enable_name.c_str(),
            this->config.lucid.gamma_enable);
    );

    const std::string gamma_name = "Gamma";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + gamma_name).c_str(),
        Arena::SetNodeValue<double>(
            device->GetNodeMap(),
            gamma_name.c_str(),
            this->config.lucid.gamma);
    );

    const std::string gain_auto_name = "GainAuto";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + gain_auto_name).c_str(),
        Arena::SetNodeValue<GenICam::gcstring>(
            device->GetNodeMap(),
            gain_auto_name.c_str(),
            this->config.lucid.gain_auto.c_str());
    );

    const std::string gain_auto_upper_limit_name = "GainAutoUpperLimit";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + gain_auto_upper_limit_name).c_str(),
        Arena::SetNodeValue<double>(
            device->GetNodeMap(),
            gain_auto_upper_limit_name.c_str(),
            this->config.lucid.gain_auto_upper_limit);
    );

    const std::string gain_auto_lower_limit_name = "GainAutoLowerLimit";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + gain_auto_lower_limit_name).c_str(),
        Arena::SetNodeValue<double>(
            device->GetNodeMap(),
            gain_auto_lower_limit_name.c_str(),
            this->config.lucid.gain_auto_lower_limit);
    );

    const std::string exposure_auto_name = "ExposureAuto";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + exposure_auto_name).c_str(),
        Arena::SetNodeValue<GenICam::gcstring>(
            device->GetNodeMap(),
            exposure_auto_name.c_str(),
            this->config.lucid.exposure_auto.c_str());
    );

    const std::string exposure_auto_damping_name = "ExposureAutoDamping";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + exposure_auto_damping_name).c_str(),
        Arena::SetNodeValue<double>(
            device->GetNodeMap(),
            exposure_auto_damping_name.c_str(),
            this->config.lucid.exposure_auto_damping);
    );

    const std::string exposure_auto_algorithm_name = "ExposureAutoAlgorithm";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + exposure_auto_algorithm_name).c_str(),
        Arena::SetNodeValue<GenICam::gcstring>(
            device->GetNodeMap(),
            exposure_auto_algorithm_name.c_str(),
            this->config.lucid.exposure_auto_algorithm.c_str());
    );

    const std::string exposure_auto_upper_limit_name = "ExposureAutoUpperLimit";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + exposure_auto_upper_limit_name).c_str(),
        Arena::SetNodeValue<double>(
            device->GetNodeMap(),
            exposure_auto_upper_limit_name.c_str(),
            this->config.lucid.exposure_auto_upper_limit);
    );

    const std::string exposure_auto_lower_limit_name = "ExposureAutoLowerLimit";
    CATCH_ARENA_EXCEPTION((std::string("setting ") + exposure_auto_lower_limit_name).c_str(),
        Arena::SetNodeValue<double>(
            device->GetNodeMap(),
            exposure_auto_lower_limit_name.c_str(),
            this->config.lucid.exposure_auto_lower_limit);
    );

    /**
     * commented out settings from black bar camera testing
     
    // factory reset
    // LOG_F(INFO, "Factory reset the camera");
    // Arena::ExecuteNode(
    //     device->GetNodeMap(),
    //     "DeviceFactoryReset"
    // );

	// enable  rolling shutter
 	Arena::SetNodeValue<GenICam::gcstring>(
		device->GetNodeMap(),
		"SensorShutterMode",
		"Rolling");

	// enable auto exposure
	// Arena::SetNodeValue<GenICam::gcstring>(
	// 	device->GetNodeMap(),
	// 	"ExposureAuto",
	// 	"Continuous");
	// // Arena::SetNodeValue<double>(
	// // 	device->GetNodeMap(),
	// // 	"ExposureTime",
	// // 	3000);
	// // Arena::SetNodeValue<double>(
	// // 	device->GetNodeMap(),
	// // 	"ExposureTime",
	// // 	3000);

	Arena::SetNodeValue<GenICam::gcstring>(
		device->GetNodeMap(),
		"DeviceLinkThroughputLimitMode",
		"On");

	GenApi::CIntegerPtr bruh = device->GetNodeMap()->GetNode("DeviceLinkThroughputLimit");
	int64_t b = bruh->GetMin();
    LOG_F(WARNING, "max %lu", b);

	Arena::SetNodeValue<int64_t>(
		device->GetNodeMap(),
		"DeviceLinkThroughputLimit",
		125000000); //max 125 000 000 min 31 250 000

	// turn on acquisition frame rate & set to max value
	Arena::SetNodeValue<bool>(
		device->GetNodeMap(),
		"AcquisitionFrameRateEnable",
		true);
	// GenApi::CFloatPtr pAcquisitionFrameRate = device->GetNodeMap()->GetNode("AcquisitionFrameRate");
	// double acquisitionFrameRate = pAcquisitionFrameRate->GetMax();
	// pAcquisitionFrameRate->SetValue(acquisitionFrameRate);



	// enable stream auto negotiate packet size
	Arena::SetNodeValue<bool>(
		device->GetTLStreamNodeMap(),
		"StreamAutoNegotiatePacketSize",
		true);

	// enable stream packet resend
	Arena::SetNodeValue<bool>(
		device->GetTLStreamNodeMap(),
		"StreamPacketResendEnable",
		true);
    */
}

std::optional<ImageData> LucidCamera::getLatestImage() {
    ReadLock lock(this->imageQueueLock);
    ImageData lastImage = this->imageQueue.front();
    this->imageQueue.pop_front();
    return lastImage;
};

std::deque<ImageData> LucidCamera::getAllImages() {
    ReadLock lock(this->imageQueueLock);
    std::deque<ImageData> outputQueue = this->imageQueue; 
    this->imageQueue = std::deque<ImageData>();
    return outputQueue;
}

// void LucidCamera::configureTrigger() 
// {
//     json configJson = config->getConfig();

//     for (auto it = configJson.begin(); it != configJson.end(); it++)
//     {
//         std::string key = it.key();


//         if (it.value().type() == json::value_t::number_unsigned)
//         {
//             int value = it.value();
//             Arena::SetNodeValue<int64_t>(
//                 this->device->GetNodeMap(),
//                 key.c_str(),
//                 value);
//         }
//         else if (it.value().type() == json::value_t::boolean)
//         {
//             bool value = it.value();
//             Arena::SetNodeValue<bool>(
//                 this->device->GetNodeMap(),
//                 key.c_str(),
//                 value);
//         }
//         else if (it.value().type() == json::value_t::number_float)
//         {

//             float value_float = it.value();
//             double value = value_float;

//             Arena::SetNodeValue<double>(
//                 this->device->GetNodeMap(),
//                 key.c_str(),
//                 value);
//         }
//         else if (it.value().type() == json::value_t::string)
//         {
//             std::string value = it.value();
//             Arena::SetNodeValue<GenICam::gcstring>(
//                 this->device->GetNodeMap(),
//                 key.c_str(),
//                 value.c_str());
//         } else {
//             std::cout << "Unkown type of varible. Skipping " << key << std::endl;
//         }
//     }
// }

bool LucidCamera::isConnected() {
    if (this->device == nullptr) {
        return false;
    }

    ReadLock lock(this->arenaDeviceLock);
    CATCH_ARENA_EXCEPTION("checking camera connection",
        return this->device->IsConnected();
    );
}

void LucidCamera::captureEvery(const std::chrono::milliseconds& interval) {
    loguru::set_thread_name("lucid camera");
    if (!this->isConnected()) {
        LOG_F(ERROR, "LUCID Camera not connected. Cannot capture photos");
        return;
    }

    this->arenaDeviceLock.lock();
    CATCH_ARENA_EXCEPTION("starting stream",
        this->device->StartStream();
    );
    this->arenaDeviceLock.unlock();

    while (this->isTakingPictures) {
        LOG_F(INFO, "Taking picture with LUCID camera");
        std::optional<ImageData> newImage = this->takePicture(this->takePictureTimeout);

        if (newImage.has_value()) {
            WriteLock lock(this->imageQueueLock);
            this->imageQueue.push_back(newImage.value());
            lock.unlock();
            LOG_F(INFO, "Took picture with LUCID camera");
        } else {
            LOG_F(ERROR, "Unable to take picture. Trying again in %ld ms", interval.count());
        }

        std::this_thread::sleep_for(interval);
    }

    this->arenaDeviceLock.lock();
    CATCH_ARENA_EXCEPTION("stopping stream",
        this->device->StopStream();
    );
    this->arenaDeviceLock.unlock();
}

std::optional<ImageData> LucidCamera::takePicture(const std::chrono::milliseconds& timeout) {
    if (!this->isConnected()) {
        LOG_F(ERROR, "LUCID Camera not connected. Cannot take picture");
        return {};
    }

    WriteLock lock(this->arenaDeviceLock);

    CATCH_ARENA_EXCEPTION("getting image",
        Arena::IImage* pImage = this->device->GetImage(timeout.count());

        static int imageCounter = 0;
        LOG_F(INFO, "Taking image: %d", imageCounter++);
        LOG_F(INFO, "Missed packet: %ld", Arena::GetNodeValue<int64_t>(device->GetTLStreamNodeMap(), "StreamMissedPacketCount"));

        LOG_F(WARNING, "Image buffer size: %lu", pImage->GetSizeOfBuffer());
        if (pImage->IsIncomplete()) {
            LOG_F(ERROR, "Image has incomplete data");
            // TODO: determine if we want to return images with incomplete data
            // return {};
        }
    );
    
    ImageData returnImg = imgConvert(pImage);

    CATCH_ARENA_EXCEPTION("freeing image buffer",
        this->device->RequeueBuffer(pImage); // frees the data of pImage
    );

    return returnImg;
}

ImageData LucidCamera::imgConvert(Arena::IImage* pImage) {
    CATCH_ARENA_EXCEPTION("converting Arena Image to OpenCV",
        Arena::IImage *pConverted = Arena::ImageFactory::Convert(
            pImage,
            BGR8);

        std::string name = "img_"+pConverted->GetTimestamp();
        std::string path = "";

        cv::Mat mat = cv::Mat(
            static_cast<int>(pConverted->GetHeight()),
            static_cast<int>(pConverted->GetWidth()),
            CV_8UC3,
            (void *)pConverted->GetData())
        .clone();
        
        // freeing underlying lucid buffers
        Arena::ImageFactory::Destroy(pConverted);

        return ImageData(name, path, mat, ImageTelemetry(0, 0, 0, 0, 0, 0, 0, 0));
    );
}

#endif // ARENA_SDK_INSTALLED
