#include "ticks/fly_waypoints.hpp"
#include "ticks/fly_search.hpp"
#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

FlyWaypointsTick::FlyWaypointsTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::FlyWaypoints) {}

std::chrono::milliseconds FlyWaypointsTick::getWait() const {
    return FLY_WAYPOINTS_TICK_WAIT;
}

Tick* FlyWaypointsTick::tick() {
    std::vector<XYZCoord> airdropBoundary = std::get<1>(this->state->mission_params.getConfig());
    std::pair<double, double> latlng = state->getMav()->latlng_deg();

    std::future<bool> takePicture = std::async(std::launch::async, [this, latlng, airdropBoundary]() {
        return this->state->getMav()->isPointInPolygon(latlng, airdropBoundary);
    });
    
    bool isMissionFinished = state->getMav()->isMissionFinished();
    
    if(isMissionFinished){
        return new FlySearchTick(this->state);
    }

    if(takePicture.get()){
            /*TODO
             *let the Camera take pictures.
             */
    }

    return nullptr;
}
