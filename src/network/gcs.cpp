#include "network/gcs.hpp"

#include <httplib.h>

#include <memory>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>

#include <loguru.hpp>

#include "core/config.hpp"
#include "core/mission_state.hpp"
#include "ticks/tick.hpp"
#include "ticks/path_gen.hpp"
#include "utilities/locks.hpp"
#include "utilities/serialize.hpp"
#include "protos/obc.pb.h"
#include "pathing/cartesian.hpp"
#include "network/gcs_routes.hpp"


GCSServer::GCSServer(uint16_t port, std::shared_ptr<MissionState> state)
    :port{port}, state{state}
{
    if (port < 1024) {
        LOG_F(ERROR, "Ports 0-1023 are reserved. Using port %d as a fallback...", DEFAULT_GCS_PORT);
        port = DEFAULT_GCS_PORT;
    }

    this->_bindHandlers();

    this->server_thread = std::thread([this, port]() {
        LOG_F(INFO, "Starting GCS HTTP server on port %d", port);
        this->server.listen("0.0.0.0", port);
        LOG_F(INFO, "GCS Server stopped on port %d", port);
    });
}

GCSServer::~GCSServer() {
    Lock lock(this->server_mut);

    this->server.stop();
    this->server_thread.join();
}

void GCSServer::_bindHandlers() {
    BIND_HANDLER(Get, mission);
    BIND_HANDLER(Post, mission);
    BIND_HANDLER(Post, airdrop);
    BIND_HANDLER(Get, path, initial);
    BIND_HANDLER(Get, path, initial, new);
    BIND_HANDLER(Post, path, initial, validate);
    BIND_HANDLER(Get, camera, status);
    BIND_HANDLER(Post, camera, start);
    BIND_HANDLER(Post, camera, stop);
    BIND_HANDLER(Post, camera, mock, start);
    BIND_HANDLER(Post, camera, mock, stop);
    BIND_HANDLER(Get, camera, capture);
    BIND_HANDLER(Get, camera, config);
    BIND_HANDLER(Post, camera, config);
}
