#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <optional>
#include "camera/lucid.hpp"
#include <fstream>
using json = nlohmann::json;

int main() 
{

    json config = json::parse(R"(
{
    "TriggerSelector":"FrameStart",
    "TriggerMode":"On",
    "TriggerSource":"Software",
    "SensorShutterMode":"Rolling",
    "ExposureAuto":"Continuous",
    "AcquisitionFrameRateEnable":true,
    "TargetBrightness":70,
    "GammaEnable":true,
    "Gamma":0.5,
    "GainAuto":"Continuous",
    "ExposureAutoDamping":1,
    "ExposureAutoAlgorithm":"Median",
    "ExposureAutoUpperLimit":500,
    "ExposureAutoLowerLimit":360,
    "GainAutoUpperLimit":10,
    "GainAutoLowerLimit":1,
    "StreamAutoNegotiatePacketSize":true,  
    "StreamPacketResendEnable":true
}
)");
    LucidCameraConfig * lucidConfig = new LucidCameraConfig(config);

    LucidCamera camera = LucidCamera(lucidConfig);
    camera.connect();
}
