#ifndef INCLUDE_CORE_MISSION_STATE_HPP_
#define INCLUDE_CORE_MISSION_STATE_HPP_

#include <memory>
#include <mutex>
#include <functional>
#include <chrono>
#include <vector>

#include "core/config.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/constants.hpp"
#include "protos/obc.pb.h"
#include "pathing/cartesian.hpp"

class Tick;

class MissionState {
 public:
    MissionState();
    ~MissionState();

    const std::optional<CartesianConverter<GPSProtoVec>>& getCartesianConverter();
    void setCartesianConverter(CartesianConverter<GPSProtoVec>);

    std::chrono::milliseconds doTick();
    // For external use, acquires the tick mutex
    // In contrast to the private version of the function,
    // which is for internal use and does not acquire
    // the mutex. This version should not be called from
    // within another function that already acquires the tick_mut
    void setTick(Tick* newTick);

    void setInitPath(std::vector<GPSCoord> init_path);
    const std::vector<GPSCoord>& getInitPath();
    bool isInitPathValidated();
    void validateInitPath();

    MissionConfig config;  // has its own mutex

 private:
    std::mutex converter_mut;
    std::optional<CartesianConverter<GPSProtoVec>> converter;

    std::mutex tick_mut;  // for reading/writing tick
    std::unique_ptr<Tick> tick;

    std::mutex init_path_mut;  // for reading/writing the initial path
    std::vector<GPSCoord> init_path;
    bool init_path_validated = false;  // true when the operator has validated the initial path

    void _setTick(Tick* newTick);  // does not acquire the tick_mut
};


#endif  // INCLUDE_CORE_MISSION_STATE_HPP_
