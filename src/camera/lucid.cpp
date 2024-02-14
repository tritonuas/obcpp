#define ARENA_SDK_INSTALLED
#ifdef ARENA_SDK_INSTALLED

#include "camera/interface.hpp"
#include "camera/lucid.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <optional>
using json = nlohmann::json;


LucidCameraConfig::LucidCameraConfig(json config) {
    this->configJson = config;
}

void LucidCameraConfig::updateConfig(json newSetting)
{
    this->configJson = newSetting;
}


// // TODO: skipped for now. This is purely for testing
// void LucidCameraConfig::updateConfigField(std::string key, std::string value)
// {
//     return;
// }

json LucidCameraConfig::getConfig() 
{
    return this->configJson;
}

json LucidCameraConfig::getConfigField(std::string name)
{
    json returnJ;
    if (this->configJson.contains(name))
    {
        returnJ[name] = this->configJson[name];
    } else {
        std::cout << "Field " + name + " doesn't exist" << std::endl;
    }

    return returnJ;
}

LucidCamera::LucidCamera(LucidCameraConfig *config)
{
    if (config != nullptr) {
        this->config = config; // huh?
    } else {
        this->config = nullptr;
    }
    this->system = Arena::OpenSystem();
    this->device = nullptr;
    this->recentPicture = nullptr;

}

int LucidCamera::connect() 
{
    try
    {
        this->system->UpdateDevices(1000);
        std::vector<Arena::DeviceInfo> deviceInfos = this->system->GetDevices();
        if (deviceInfos.size() == 0)
        {
            std::cout << "\nNo camera connected\nPress enter to complete\n";
            std::getchar();
            return -1;
        }
        this->device = this->system->CreateDevice(deviceInfos[0]);

        // run configuration
        std::cout << "Trigger configuration start\n\n";
        configureTrigger();
        std::cout << "\nTrigger configured finished\n";

        std::cout << "Verifying connection\n\n";

        if (!verifyConnection()) 
        {
            std::cout << "Connection fail!\n\n";
            throw std::exception();
        }

        // TODO: add to destructor
        // pSystem->DestroyDevice(pDevice);
        // Arena::CloseSystem(pSystem);
    }
    catch (GenICam::GenericException &ge)
    {
        std::cout << "\nGenICam exception thrown: " << ge.what() << "\n";
        return -1;
    }
    catch (std::exception &ex)
    {
        std::cout << "Standard exception thrown: " << ex.what() << "\n";
        return -1;
    }
    catch (...)
    {
        std::cout << "Unexpected exception thrown\n";
        return -1;
    }
}

void LucidCamera::configureTrigger() 
{
    json configJson = config->getConfig();

    for (auto it = configJson.begin(); it != configJson.end(); it++)
    {
        std::string key = it.key();

        if (it.value().type() == json::value_t::number_integer)
        {
            int value = it.value();
            Arena::SetNodeValue<int64_t>(
                this->device->GetNodeMap(),
                key.c_str(),
                value);
        }
        else if (it.value().type() == json::value_t::boolean)
        {
            bool value = it.value();
            Arena::SetNodeValue<bool>(
                this->device->GetNodeMap(),
                key.c_str(),
                value);
        }
        else if (it.value().type() == json::value_t::number_float)
        {

            float value_float = it.value();
            double value = value_float;

            Arena::SetNodeValue<double>(
                this->device->GetNodeMap(),
                key.c_str(),
                value);
        }
        else if (it.value().type() == json::value_t::string)
        {
            std::string value = it.value();
            Arena::SetNodeValue<GenICam::gcstring>(
                this->device->GetNodeMap(),
                key.c_str(),
                value.c_str());
        } else {
            std::cout << "Unkown type of varible. Skipping " << key << std::endl;
        }
    }
}

bool LucidCamera::verifyConnection()
{
    return this->device->IsConnected();
}

ImageData * LucidCamera::takePicture()
{
    return nullptr;
}

ImageData *  LucidCamera::getLastPicture()
{
    return nullptr;
}

#endif // ARENA_SDK_INSTALLED
