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
#include "protos/obc.pb.h"
#include "pathing/cartesian.hpp"
#include "ticks/ids.hpp"
#include "ticks/message.hpp"
#include "network/mavlink.hpp"
#include "network/airdrop_client.hpp"

class Tick;

template<typename T>
class TickRef {
 public:
    TickRef(Tick& tick, std::mutex& mut) {
        this->mut = mut;
        this->mut.lock();

        try {
            this->tick = dynamic_cast<T&>(tick);
        } catch (std::bad_cast err) {
            LOG_F(ERROR, "Bad TickRef creation: %s", err.what)
        }
    }
    ~TickRef() {
        this->mut.unlock();
    }

    T& tick;
 private:
    std::mutex& mut;
};

class MissionState {
 public:
    MissionState();
    ~MissionState();

    const std::optional<CartesianConverter<GPSProtoVec>>& getCartesianConverter();
    void setCartesianConverter(CartesianConverter<GPSProtoVec>);

    std::chrono::milliseconds doTick();
    TickID getTickID();
    bool sendTickMsg(TickMessage msg); // return false if TickMessage not addressed to curr tick
    std::optional<TickMessage> recvTickMsg();

    void setInitPath(std::vector<GPSCoord> init_path);
    const std::vector<GPSCoord>& getInitPath();

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
    std::unique_ptr<Tick> tick;
    std::mutex tick_msgs_mut;
    std::queue<TickMessage> tick_msgs;

    std::mutex init_path_mut;  // for reading/writing the initial path
    std::vector<GPSCoord> init_path;

    std::shared_ptr<MavlinkClient> mav;
    std::shared_ptr<AirdropClient> airdrop;
    

    void _setTick(Tick* newTick);  // does not acquire the tick_mut
};


#endif  // INCLUDE_CORE_MISSION_STATE_HPP_
