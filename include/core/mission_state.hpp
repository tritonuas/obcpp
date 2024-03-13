#ifndef INCLUDE_CORE_MISSION_STATE_HPP_
#define INCLUDE_CORE_MISSION_STATE_HPP_

#include <memory>
#include <mutex>
#include <functional>
#include <chrono>
#include <vector>
#include <optional>
#include <queue>

#include "core/mission_config.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/constants.hpp"
#include "utilities/locks.hpp"
#include "utilities/lockptr.hpp"
#include "utilities/logging.hpp"
#include "protos/obc.pb.h"
#include "pathing/cartesian.hpp"
#include "ticks/ids.hpp"
#include "network/mavlink.hpp"
#include "network/airdrop_client.hpp"

class Tick;

class MissionState {
 public:
    MissionState();
    ~MissionState();

    const std::optional<CartesianConverter<GPSProtoVec>>& getCartesianConverter();
    void setCartesianConverter(CartesianConverter<GPSProtoVec>);

    std::chrono::milliseconds doTick();
    TickID getTickID();

    void setTick(Tick* newTick);

    void setInitPath(std::vector<GPSCoord> init_path);
    const std::vector<GPSCoord>& getInitPath();

    /*
     * Gets a locking reference to the underlying tick for the given tick subclass T.
     * 
     * Needs to be defined in the header file unless we want to manually list out
     * template derivations.
     */
    template <typename T>
    std::optional<LockPtr<T>> getTickLockPtr() {
        try {
            return LockPtr(std::dynamic_pointer_cast<T>(this->tick), this->tick_mut);
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

    MissionConfig config;  // has its own mutex
 private:
    std::mutex converter_mut;
    std::optional<CartesianConverter<GPSProtoVec>> converter;

    std::mutex tick_mut;  // for reading/writing tick
    std::shared_ptr<Tick> tick;

    std::mutex init_path_mut;  // for reading/writing the initial path
    std::vector<GPSCoord> init_path;

    std::shared_ptr<MavlinkClient> mav;
    std::shared_ptr<AirdropClient> airdrop;

    void _setTick(Tick* newTick);  // does not acquire the tick_mut
};

#endif  // INCLUDE_CORE_MISSION_STATE_HPP_
