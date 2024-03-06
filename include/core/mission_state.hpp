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

    template<typename TickSubClass>
    bool sendTickMsg(TickSubClass::Message msg) {
        TickSubClass* tick = dynamic_cast<TickSubClass*>(this->tick.get());
        if (tick == nullptr) {
            return false;
        }

        Lock lock(this->tick_msgs_mut);
        this->tick_msgs.push(static_cast<int>(msg));
        return true;
    }

    // Would have liked to use std::optional for return type, but this does not work
    // because TickSubClass::Message can't be passed into the std::optional as a further
    // template argument because it isn't a defined type at this point
    template<typename TickSubClass>
    bool recvTickMsg(TickSubClass::Message& msg) {
        Lock lock(this->tick_msgs_mut);
        if (this->tick_msgs.empty()) {
            return false;
        }

        int msg_int = this->tick_msgs.front();
        this->tick_msgs.pop();
        msg = static_cast<TickSubClass::Message>(msg_int);
        return true;
    }

 private:
    std::mutex converter_mut;
    std::optional<CartesianConverter<GPSProtoVec>> converter;

    std::mutex tick_mut;  // for reading/writing tick
    std::unique_ptr<Tick> tick;
    std::mutex tick_msgs_mut;
    std::queue<int> tick_msgs;

    std::mutex init_path_mut;  // for reading/writing the initial path
    std::vector<GPSCoord> init_path;

    std::shared_ptr<MavlinkClient> mav;
    std::shared_ptr<AirdropClient> airdrop;

    void _setTick(Tick* newTick);  // does not acquire the tick_mut
};

#endif  // INCLUDE_CORE_MISSION_STATE_HPP_
