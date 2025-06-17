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

#include "camera/picamera.hpp"
#include "camera/interface.hpp"
#include "network/mavlink.hpp"
#include "utilities/locks.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/common.hpp"

using json = nlohmann::json;

PiCamera::PiCamera(CameraConfig config) : 
    CameraInterface(config){

}

void PiCamera::connect() {

}

PiCamera::~PiCamera(){

}

