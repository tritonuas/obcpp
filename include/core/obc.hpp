#ifndef INCLUDE_CORE_OBC_HPP_
#define INCLUDE_CORE_OBC_HPP_

#include <string>
#include <memory>
#include <cstdint>

#include "core/mission_config.hpp"
#include "core/mission_state.hpp"
#include "network/gcs.hpp"
#include "network/mavlink.hpp"
#include "utilities/OBCConfig.hpp"

/*
 *  The highest level class for the entire OBC
 *  This contains all of the mission state, mission config, etc...
 */
class OBC {
 public:
    explicit OBC(OBCConfig config);

    void run();

 private:
    std::shared_ptr<MissionState> state;

    std::unique_ptr<GCSServer> gcs_server;

    std::thread connectMavThread;
    std::thread connectAirdropThread;

    void connectMavlink(const char* mavlink_url);
    void connectAirdrop();
};

#endif  // INCLUDE_CORE_OBC_HPP_
