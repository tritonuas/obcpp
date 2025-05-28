#include <memory>
#include "ticks/refueling.hpp"
#include "ticks/ids.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/active_takeoff.hpp"
#include "utilities/constants.hpp"

RefuelingTick::RefuelingTick(std::shared_ptr<MissionState> state)
    : Tick(state, TickID::Refueling) {}

std::chrono::milliseconds RefuelingTick::getWait() const { return REFUELING_TICK_WAIT; }

Tick* RefuelingTick::tick() {
     if (state.get()->getMav()->isArmed()) {
         return new MavUploadTick(state, new WaitForTakeoffTick(state), state->getInitPath(), false);
    }
    
    // return new MavUploadTick(state, new WaitForTakeoffTick(state), state->getInitPath(), false);
}