#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <optional>
#include "camera/lucid.hpp"
#include <fstream>
using json = nlohmann::json;

int main() 
{
    std::ifstream f("./util/config.json");
    json config = json::parse(f);
    LucidCameraConfig * lucidConfig = new LucidCameraConfig(config);

    LucidCamera camera = LucidCamera(lucidConfig);
    camera.connect();
}
