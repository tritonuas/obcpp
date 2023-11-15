#ifndef CORE_STATES_HPP_
#define CORE_STATES_HPP_

#include <memory>
#include <mutex>
#include <functional>
#include <chrono>

#include "core/config.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/constants.hpp"

class Tick;

class MissionState {
    public:
        MissionState();
        ~MissionState();

        void init();

        const MissionConfig& getConfig();

        std::chrono::milliseconds doTick();
        void setTick(Tick* newTick);
    private: 
        MissionConfig config; // has its own mutex

        std::mutex tick_mut; // for reading/writing tick
        std::unique_ptr<Tick> tick;

        // other state...
};


#endif // CORE_STATES_HPP_