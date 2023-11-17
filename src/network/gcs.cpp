#include <memory>
#include <cstdint>
#include <functional>

#include <httplib.h>

#include "network/gcs.hpp"
#include "core/config.hpp"
#include "core/states.hpp"
#include "utilities/locks.hpp"

GCSServer::GCSServer(uint16_t port, std::shared_ptr<MissionState> state)
    :port{port}, state{state}
{
    if (port < 1024) {
        std::cerr << "Ports 0-1023 are reserved. Using port " << DEFAULT_GCS_PORT << " as a fallback..." << std::endl;
        port = DEFAULT_GCS_PORT;
    }

    this->_bindHandlers();

    this->server_thread = std::thread([this, port]() {
        this->server.listen("0.0.0.0", port);
    });
}

GCSServer::~GCSServer() {
    Lock lock(this->server_mut);

    this->server.stop();
}

void GCSServer::_bindHandlers() {
    #define HANDLE(name) (std::bind_front(&GCSServer::name, this))

    server.Get("/mission", HANDLE(_getMission));
}

void GCSServer::_getMission(const httplib::Request& request, httplib::Response& response) {
    response.set_content("Mission poggers!", "text/plain");
}
