#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <optional>
#include "camera/lucid.hpp"
#include <fstream>
using json = nlohmann::json;

int main() 
{
    std::ifstream f("home/tritonuas/boris-lucid-obcpp/tests/integration/util/config.json");
    std::cout << "Reading json file." << std::endl;
    std::cout << f.rdbuf() << std::endl;
    json config = json::parse(f);
    LucidCameraConfig * lucidConfig = new LucidCameraConfig(config);

    LucidCamera camera = LucidCamera(lucidConfig);
    camera.connect();
}
