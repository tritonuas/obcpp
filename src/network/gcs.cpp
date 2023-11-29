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
        std::cout << "Starting GCS Server on port " << port << std::endl;
        this->server.listen("0.0.0.0", port);
        std::cout << "GCS Server stopped on port " << port << std::endl;
    });
}

GCSServer::~GCSServer() {
    Lock lock(this->server_mut);

    this->server.stop();
    this->server_thread.join();
}

void GCSServer::_bindHandlers() {
    #define HANDLE(name) (std::bind_front(&GCSServer::name, this))

    server.Get("/mission", HANDLE(_getMission));
    server.Post("/mission", HANDLE(_postMission));
    server.Post("/airdrop", HANDLE(_postAirdropTargets));
    server.Get("/path/initial", HANDLE(_getPathInitial));
    server.Get("/path/initial/new", HANDLE(_getPathInitialNew));
    server.Get("/camera/status", HANDLE(_getCameraStatus));
    server.Post("/camera/start", HANDLE(_postCameraStart));
    server.Post("/camera/mock/start", HANDLE(_postCameraMockStart));
    server.Post("/camera/stop", HANDLE(_postCameraStop));
    server.Post("/camera/mock/stop", HANDLE(_postCameraMockStop));
    server.Get("/camera/capture", HANDLE(_getCameraCapture));
    server.Get("/camera/config", HANDLE(_getCameraConfig));
    server.Post("/camera/config", HANDLE(_postCameraConfig));
}

void GCSServer::_getMission(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: return back mission config!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_postMission(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: upload mission config!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_postAirdropTargets(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: upload airdrop targets!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_getPathInitial(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: get cached path and return back!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_getPathInitialNew(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: calculate path using RRT, replace cached path, and return back!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_postPathInitialValidate(const httplib::Request& request, httplib::Response& response) {
    if (state->getInitPath().empty()) {
        response.set_content("Error: No initial path generated.", "text/plain");
        response.status = HTTPStatus::BAD_REQUEST;
    } else {
        response.set_content("Current initial path validated.", "text/plain");
        response.status = HTTPStatus::OK;
    }
}

void GCSServer::_getCameraStatus(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: get camera status and return back!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_postCameraStart(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: start taking real images with camera!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_postCameraMockStart(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: start taking mock images with mock camera!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_postCameraStop(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: stop taking real images with camera!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_postCameraMockStop(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: stop taking mock images with mock camera!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_getCameraCapture(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: take a singular image with the camera and return as jpeg!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_getCameraConfig(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: get camera config and return back!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_postCameraConfig(const httplib::Request& request, httplib::Response& response) {
    response.set_content("TODO: upload camera config and return back status!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}