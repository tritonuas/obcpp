#ifndef INCLUDE_CORE_MISSION_STATE_HPP_
#define INCLUDE_CORE_MISSION_STATE_HPP_

#include <array>
#include <chrono>
#include <cmath>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <unordered_set>
#include <vector>

#include "camera/interface.hpp"
#include "core/mission_parameters.hpp"
#include "cv/aggregator.hpp"
#include "cv/utilities.hpp"
#include "network/airdrop_client.hpp"
#include "network/mavlink.hpp"
#include "pathing/cartesian.hpp"
#include "pathing/mission_path.hpp"
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
    explicit MissionState(OBCConfig config);
    ~MissionState();

    const std::optional<CartesianConverter<GPSProtoVec>>& getCartesianConverter();
    void setCartesianConverter(CartesianConverter<GPSProtoVec>);

    std::chrono::milliseconds doTick();
    TickID getTickID();

    void setTick(Tick* newTick);

    void setInitPath(const MissionPath& init_path);
    MissionPath getInitPath();

    void setCoveragePath(const MissionPath& coverage_path);
    MissionPath getCoveragePath();

    void setAirdropPath(const MissionPath& airdrop_path);
    MissionPath getAirdropPath();

    void markAirdropAsDropped(AirdropIndex airdrop);
    std::unordered_set<AirdropIndex> getDroppedAirdrops();

    /*
     * Gets a locking reference to the underlying tick for the given tick subclass T.
     *
     * Needs to be defined in the header file unless we want to manually list out
     * template derivations.
     */
    template <typename T>
    std::optional<LockPtr<T>> getTickLockPtr() {
        auto ptr = std::dynamic_pointer_cast<T>(this->tick);
        if (ptr != nullptr) {
            return LockPtr(ptr, &this->tick_mut);
        } else {
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

    // Getters and setters for mapping status.
    bool getMappingIsDone();
    void setMappingIsDone(bool isDone);

    MissionParameters mission_params;  // has its own mutex

    OBCConfig config;

    std::optional<airdrop_t> next_airdrop_to_drop;

 private:
    std::mutex converter_mut;
    std::optional<CartesianConverter<GPSProtoVec>> converter;

    std::mutex tick_mut;  // for reading/writing tick
    std::shared_ptr<Tick> tick;

    std::mutex init_path_mut;  // for reading/writing the initial path
    MissionPath init_path;
    std::mutex coverage_path_mut;  // for reading/writing the coverage path
    MissionPath coverage_path;
    std::mutex airdrop_path_mut;
    MissionPath airdrop_path;

    std::mutex dropped_airdrops_mut;
    std::unordered_set<AirdropIndex> dropped_airdrops;

    std::shared_ptr<MavlinkClient> mav;
    std::shared_ptr<AirdropClient> airdrop;
    std::shared_ptr<CVAggregator> cv;
    std::shared_ptr<CameraInterface> camera;

    std::mutex cv_mut;
    // Represents a single detected target used in pipeline
    std::vector<DetectedTarget> cv_detected_targets;
    // Gives an index into cv_detected_targets, and specifies that that bottle is matched
    // with the detected_target specified by the index
    std::array<size_t, NUM_AIRDROPS> cv_matches;

    bool mappingIsDone;

    void _setTick(Tick* newTick);  // does not acquire the tick_mut
};

#endif  // INCLUDE_CORE_MISSION_STATE_HPP_
