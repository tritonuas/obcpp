#ifndef CORE_STATES_HPP_
#define CORE_STATES_HPP_

#include <memory>
#include <mutex>
#include <functional>
#include <chrono>
#include <vector>

#include "core/config.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/constants.hpp"
#include "obc.pb.h"

class Tick;

class MissionState {
    public:
        MissionState();
        ~MissionState();

        void init();

        const MissionConfig& getConfig();

        std::chrono::milliseconds doTick();
        void setTick(Tick* newTick);

        void setInitPath(std::vector<GPSCoord> init_path);
        const std::vector<GPSCoord>& getInitPath();
        bool isInitPathValidated();
    private: 
        MissionConfig config; // has its own mutex

        std::mutex tick_mut; // for reading/writing tick
        std::unique_ptr<Tick> tick;

        std::mutex init_path_mut;  // for reading/writing the initial path
        std::vector<GPSCoord> init_path;
        bool init_path_validated = false; // true when the operator has validated the initial path
};


#endif // CORE_STATES_HPP_