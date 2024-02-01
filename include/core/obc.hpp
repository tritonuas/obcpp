#ifndef INCLUDE_CORE_OBC_HPP_
#define INCLUDE_CORE_OBC_HPP_

#include <string>
#include <memory>
#include <cstdint>

#include "core/config.hpp"
#include "core/mission_state.hpp"
#include "network/gcs.hpp"

/*
 *  The highest level class for the entire OBC
 *  This contains all of the mission state, mission config, etc...
 */
class OBC {
 public:
    explicit OBC(uint16_t gcs_port);

    void run();

 private:
    std::shared_ptr<MissionState> state;

    std::unique_ptr<GCSServer> gcs_server;
};

#endif  // INCLUDE_CORE_OBC_HPP_
