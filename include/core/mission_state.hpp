#ifndef INCLUDE_CORE_MISSION_STATE_HPP_
#define INCLUDE_CORE_MISSION_STATE_HPP_

#include <array>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

#include "camera/interface.hpp"
#include "core/mission_parameters.hpp"
#include "cv/aggregator.hpp"
#include "cv/utilities.hpp"
#include "network/airdrop_client.hpp"
#include "network/mavlink.hpp"
#include "pathing/cartesian.hpp"
#include "protos/obc.pb.h"
#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/lockptr.hpp"
#include "utilities/locks.hpp"
#include "utilities/logging.hpp"
#include "utilities/obc_config.hpp"

class Tick;

class MissionState {
 public:
    MissionState(OBCConfig config);
    ~MissionState();

    const std::optional<CartesianConverter<GPSProtoVec>>& getCartesianConverter();
    void setCartesianConverter(CartesianConverter<GPSProtoVec>);

    std::chrono::milliseconds doTick();
    TickID getTickID();

    void setTick(Tick* newTick);

    void setInitPath(std::vector<GPSCoord> init_path);
    const std::vector<GPSCoord>& getInitPath();

    void setSearchPath(std::vector<GPSCoord> search_path);
    const std::vector<GPSCoord>& getSearchPath();

    /*
     * Gets a locking reference to the underlying tick for the given tick subclass T.
     *
     * Needs to be defined in the header file unless we want to manually list out
     * template derivations.
     */
    template <typename T>
    std::optional<LockPtr<T>> getTickLockPtr() {
        try {
            return LockPtr(std::dynamic_pointer_cast<T>(this->tick), &this->tick_mut);
        } catch (std::bad_cast ex) {
            LOG_F(ERROR, "Error creating TickLockRef: %s", ex.what());
            return {};
        }
    }

    /*
     * Gets a shared_ptr to the mavlink client.
     * IMPORTANT: need to check that the pointer is not nullptr
     * before accessing, to make sure the connection has already
     * been established
     */
    std::shared_ptr<MavlinkClient> getMav();
    void setMav(std::shared_ptr<MavlinkClient> mav);

    /*
     * Gets a shared_ptr to the airdrop client.
     * IMPORTANT: need to check that the pointer is not nullptr
     * before accessing, to make sure the connection has already
     * been established
     */
    std::shared_ptr<AirdropClient> getAirdrop();
    void setAirdrop(std::shared_ptr<AirdropClient> airdrop);

    /*
     * Gets a shared_ptr to the CVAggregator, which lets you
     * run the CV pipeline on images in background threads.
     */
    std::shared_ptr<CVAggregator> getCV();
    void setCV(std::shared_ptr<CVAggregator> cv);

    /*
     * Gets a shared_ptr to the camera client, which lets you
     * take photos of ground targets.
     */
    std::shared_ptr<CameraInterface> getCamera();
    void setCamera(std::shared_ptr<CameraInterface> camera);

    MissionParameters mission_params;  // has its own mutex

    OBCConfig config;

 private:
    std::mutex converter_mut;
    std::optional<CartesianConverter<GPSProtoVec>> converter;

    std::mutex tick_mut;  // for reading/writing tick
    std::shared_ptr<Tick> tick;

    std::mutex init_path_mut;  // for reading/writing the initial path
    std::mutex search_path_mut;  // for reading/writing the search path
    std::vector<GPSCoord> init_path;
    std::vector<GPSCoord> search_path;

    std::shared_ptr<MavlinkClient> mav;
    std::shared_ptr<AirdropClient> airdrop;
    std::shared_ptr<CVAggregator> cv;
    std::shared_ptr<CameraInterface> camera;

    std::mutex cv_mut;
    std::vector<DetectedTarget> cv_detected_targets;
    // Gives an index into cv_detected_targets, and specifies that that bottle is matched
    // with the detected_target specified by the index
    std::array<size_t, NUM_AIRDROP_BOTTLES> cv_matches;

    void _setTick(Tick* newTick);  // does not acquire the tick_mut
};

#endif  // INCLUDE_CORE_MISSION_STATE_HPP_
