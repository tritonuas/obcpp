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

    try {
        this->system = Arena::OpenSystem();

        while (true) {
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

            LOG_F(ERROR,"Lucid camera connection failed! Retrying in %ld ms", this->connectionRetry.count());
            std::this_thread::sleep_for(this->connectionRetry);

        }
    }
    catch (GenICam::GenericException &ge) {
        LOG_F(ERROR, "GenICam exception thrown: %s", ge.what());
        throw ge;
    }
    catch (std::exception &ex) {
        LOG_F(ERROR, "Standard exception thrown: %s", ex.what());
        throw ex;
    }
    catch (...) {
        LOG_F(ERROR, "Unexpected exception thrown:");
        throw std::exception();
    }

    this->configureDefaults();
}

LucidCamera::~LucidCamera() {
    // aquire locks to Arena System and Device
    WriteLock systemLock(this->arenaSystemLock);
    WriteLock deviceLock(this->arenaDeviceLock);

    this->system->DestroyDevice(this->device);
    Arena::CloseSystem(this->system);
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

void LucidCamera::configureDefaults() {
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
    return this->device->IsConnected();
}

void LucidCamera::captureEvery(const std::chrono::milliseconds& interval) {
    if (!this->isConnected()) {
        LOG_F(ERROR, "LUCID Camera not connected. Cannot capture photos");
        return;
    }

    this->arenaDeviceLock.lock();
    this->device->StartStream();
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
    this->device->StopStream();
    this->arenaDeviceLock.unlock();
}

std::optional<ImageData> LucidCamera::takePicture(const std::chrono::milliseconds& timeout) {
    if (!this->isConnected()) {
        LOG_F(ERROR, "LUCID Camera not connected. Cannot take picture");
        return {};
    }

    WriteLock lock(this->arenaDeviceLock);

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

    
    ImageData returnImg = imgConvert(pImage);

    this->device->RequeueBuffer(pImage); // frees the data of pImage

    return returnImg;
}

ImageData LucidCamera::imgConvert(Arena::IImage* pImage) {
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
}

#endif // ARENA_SDK_INSTALLED
