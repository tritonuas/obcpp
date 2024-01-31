#ifndef INCLUDE_CORE_STATES_HPP_
#define INCLUDE_CORE_STATES_HPP_

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

   const std::optional<CartesianConverterProto>& getCartesianConverter();
   void setCartesianConverter(CartesianConverterProto);

   std::chrono::milliseconds doTick();
   void setTick(Tick* newTick);

   void setInitPath(std::vector<GPSCoord> init_path);
   const std::vector<GPSCoord>& getInitPath();
   bool isInitPathValidated();

   MissionConfig config;  // has its own mutex
 private:
   std::mutex converter_mut;
   std::optional<CartesianConverterProto> converter;

   std::mutex tick_mut;  // for reading/writing tick
   std::unique_ptr<Tick> tick;

   std::mutex init_path_mut;  // for reading/writing the initial path
   std::vector<GPSCoord> init_path;
   bool init_path_validated = false;  // true when the operator has validated the initial path
};


#endif  // INCLUDE_CORE_STATES_HPP_
