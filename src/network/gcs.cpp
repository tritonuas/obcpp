#include <memory>

#include <httplib.h>

#include "network/gcs.hpp"
#include "core/config.hpp"
#include "core/states.hpp"

GCSServer::GCSServer(std::shared_ptr<MissionState> state) {
    this->state = state;

    this->server_thread = std::thread([this]() {this->_listen();});
}

GCSServer::~GCSServer() {
    // TODO: develop a mechanism to stop the running gcs thread
    // which is inside this->server_thread
}

void GCSServer::_listen() {
    httplib::Server svr;
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });
    svr.listen("0.0.0.0", 8080);
}