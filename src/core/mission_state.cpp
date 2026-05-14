#include "core/mission_state.hpp"

#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>

#include "camera/interface.hpp"
#include "camera/mock.hpp"
#include "core/mission_parameters.hpp"
#include "cv/aggregator.hpp"
#include "cv/pipeline.hpp"
#include "network/airdrop_client.hpp"
#include "network/mavlink.hpp"
#include "ticks/tick.hpp"
#include "utilities/locks.hpp"
#include "utilities/logging.hpp"
#include "utilities/obc_config.hpp"
#include "pathing/environment.hpp"
#include "utilities/common.hpp"
#include "ticks/ids.hpp"

using namespace std::chrono_literals;

MissionState::MissionState(OBCConfig config) : config(config) {}

// Need to explicitly define now that Tick is no longer an incomplete class
// See:
// https://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
MissionState::~MissionState() = default;

const std::optional<CartesianConverter<GPSProtoVec>>& MissionState::getCartesianConverter() {
    Lock lock(this->converter_mut);

    return this->converter;
}

void MissionState::setCartesianConverter(CartesianConverter<GPSProtoVec> new_converter) {
    Lock lock(this->converter_mut);

    this->converter = new_converter;
}

std::chrono::milliseconds MissionState::doTick() {
    Lock lock(this->tick_mut);

    Tick* newTick = this->tick->tick();
    if (newTick) {
        this->_setTick(newTick);
    }

    return this->tick->getWait();
}

void MissionState::setTick(Tick* newTick) {
    Lock lock(this->tick_mut);

    this->_setTick(newTick);
}

void MissionState::_setTick(Tick* newTick) {
    std::string old_tick_name = (this->tick) ? this->tick->getName() : "Null";
    std::string new_tick_name = (newTick) ? newTick->getName() : "Null";

    LOG_F(INFO, "%s -> %s", old_tick_name.c_str(), new_tick_name.c_str());

    this->tick.reset(newTick);
    if (newTick != nullptr) {
        this->tick->init();
    }
}

TickID MissionState::getTickID() {
    Lock lock(this->tick_mut);
    return this->tick->getID();
}

void MissionState::setInitPath(const MissionPath& init_path) {
    Lock lock(this->init_path_mut);
    this->init_path = init_path;
}

MissionPath MissionState::getInitPath() {
    Lock lock(this->init_path_mut);
    return this->init_path;
}

void MissionState::setCoveragePath(const MissionPath& coverage_path) {
    Lock lock(this->coverage_path_mut);
    this->coverage_path = coverage_path;
}

MissionPath MissionState::getCoveragePath() {
    Lock lock(this->coverage_path_mut);
    return this->coverage_path;
}

void MissionState::setAirdropPath(const MissionPath& airdrop_path) {
    Lock lock(this->airdrop_path_mut);
    this->airdrop_path = airdrop_path;
}



void MissionState::zoneHandler(const std::chrono::milliseconds& interval,
                        std::shared_ptr<MavlinkClient> mavlinkClient){
    //TODO: find a way to handle stopping the while loop when stopThread hits
    std::chrono::milliseconds last_photo_time = getUnixTime_ms();
    while(this->cameraThreadActive){
        auto [lat_deg, lng_deg] =this->getMav()->latlng_deg();
        double altitude_agl_m = this->getMav()->altitude_agl_m();
        GPSCoord current_pos = makeGPSCoord(lat_deg, lng_deg, altitude_agl_m);
        // Get the CartesianConverter (which is already initialized from mission boundaries)
        auto converter = this->getCartesianConverter();
        if (converter) {
            // Convert GPS to local XYZ coords
            XYZCoord current_xyz = converter->toXYZ(current_pos);
            // Get the airdrop boundary polygon
            Polygon airdrop_boundary = this->mission_params.getAirdropBoundary();
            // Check if we're inside the airdrop zone
            bool in_zone = Environment::isPointInPolygon(airdrop_boundary, current_xyz);
            if(in_zone){
                auto now = getUnixTime_ms();
                if ((now - last_photo_time) >= 300ms) {
                    auto photo = this->getCamera()->takePicture(100ms, this->getMav());
                    if (this->config.camera.save_images_to_file) {
                        photo->saveToFile(this->config.camera.save_dir);
                    }

                    if (photo.has_value()&&((this->getTickID()==TickID::FlySearch)||
                        (this->getTickID()==TickID::CVLoiter))) {
                        // Update the last photo time
                        // Run the pipeline on the photo
                        this->getCV()->runPipeline(photo.value());
                    }
                    last_photo_time = getUnixTime_ms();
                }
            }
        }
        std::this_thread::sleep_for(interval);
    }
}
void MissionState::initThread(const std::chrono::milliseconds& interval,
    std::shared_ptr<MavlinkClient> mavlinkClient){
    this->cameraThreadActive=true;
    this->captureThread = std::thread([this, interval, mavlinkClient]() {
        this->zoneHandler(interval, mavlinkClient);
    });

}
void MissionState::stopThread(){
    this->captureThread.join();
    this->cameraThreadActive = false;
}

MissionPath MissionState::getAirdropPath() {
    Lock lock(this->airdrop_path_mut);
    return this->airdrop_path;
}

void MissionState::markAirdropAsDropped(AirdropType airdrop) {
    Lock lock(this->dropped_airdrops_mut);
    this->dropped_airdrops.insert(airdrop);
}

std::unordered_set<AirdropType> MissionState::getDroppedAirdrops() {
    Lock lock(this->dropped_airdrops_mut);
    return this->dropped_airdrops;
}

std::shared_ptr<MavlinkClient> MissionState::getMav() { return this->mav; }

void MissionState::setMav(std::shared_ptr<MavlinkClient> mav) { this->mav = mav; }

std::shared_ptr<AirdropClient> MissionState::getAirdrop() { return this->airdrop; }

void MissionState::setAirdrop(std::shared_ptr<AirdropClient> airdrop) { this->airdrop = airdrop; }

std::shared_ptr<CVAggregator> MissionState::getCV() { return this->cv; }

void MissionState::setCV(std::shared_ptr<CVAggregator> cv) { this->cv = cv; }

std::shared_ptr<CameraInterface> MissionState::getCamera() { return this->camera; }

void MissionState::setCamera(std::shared_ptr<CameraInterface> camera) { this->camera = camera; }

bool MissionState::getMappingIsDone() { return this->mappingIsDone; }

void MissionState::setMappingIsDone(bool isDone) { this->mappingIsDone = isDone; }


