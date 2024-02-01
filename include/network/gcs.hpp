#ifndef INCLUDE_NETWORK_GCS_HPP_
#define INCLUDE_NETWORK_GCS_HPP_

#include <httplib.h>

#include <thread>
#include <memory>
#include <mutex>
#include <cstdint>
#include <optional>

#include "core/config.hpp"
#include "core/mission_state.hpp"
#include "protos/obc.pb.h"


class GCSServer {
 public:
    GCSServer(uint16_t port, std::shared_ptr<MissionState> state);

    ~GCSServer();

 private:
    httplib::Server server;
    std::mutex server_mut;
    std::thread server_thread;
    uint16_t port;

    std::shared_ptr<MissionState> state;

    // Handler Functions
    void _bindHandlers();  // bind all the handlers to the server object

};

#endif  // INCLUDE_NETWORK_GCS_HPP_