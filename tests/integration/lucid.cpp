#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <optional>
#include "camera/lucid.hpp"
using json = nlohmann::json;

int main() 
{
    LucidCamera camera = LucidCamera(nullptr);
    camera.connect();
}
