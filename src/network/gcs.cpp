#include "network/gcs.hpp"

#include <google/protobuf/util/json_util.h>
#include <httplib.h>
#include <loguru.hpp>

#include <memory>
#include <cstdint>
#include <functional>

#include "core/config.hpp"
#include "core/states.hpp"
#include "utilities/locks.hpp"
#include "protos/obc.pb.h"
#include "pathing/cartesian.hpp"

#define LOG_REQUEST(method, route, request) \
    LOG_SCOPE_F(INFO, "%s %s", method, route); \
    LOG_F(INFO, "User-Agent: %s", request.get_header_value("User-Agent").c_str());

// TODO: macro to overload the macros

#define LOG_RESPONSE(LOG_LEVEL, msg, response_code) \
    LOG_F(LOG_LEVEL, "%s", msg); \
    LOG_F(LOG_LEVEL, "HTTP %d: %s", response_code, HTTP_STATUS_TO_STRING.at(response_code)); \
    response.set_content(msg, mime); \
    response.status = response_code;

#define LOG_RESPONSE(LOG_LEVEL, msg, response_code, body, mime) \
    LOG_F(LOG_LEVEL, "%s", msg); \
    LOG_F(LOG_LEVEL, "HTTP %d: %s", response_code, HTTP_STATUS_TO_STRING.at(response_code)); \
    if (msg != body) LOG_F(LOG_LEVEL, "%s", body); \
    response.set_content(body, mime::plaintext); \
    response.status = response_code;

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
    LOG_REQUEST("GET", "/mission", request);

    if (this->uploaded_mission) {
        std::string output;
        google::protobuf::util::MessageToJsonString(this->uploaded_mission.value(), &output);


        LOG_RESPONSE(INFO, "Returning valid mission", HTTPStatus::OK, output.c_str(), mime::json);
    } else {
        LOG_RESPONSE(WARNING, "No mission uploaded", HTTPStatus::BAD_REQUEST);
    }
}

void GCSServer::_postMission(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("POST", "/mission", request);
    
    Mission mission;
    google::protobuf::util::JsonStringToMessage(request.body, &mission);

    // TODO: add checks for the mission

    // Update the cartesian converter to be centered around the new flight boundary
    this->state->setCartesianConverter(CartesianConverterProto(mission.flightboundary()));
    // Create the cartesian polygons for this new mission
    // and store in the mission state
    auto converter = state->getCartesianConverter().value();
    this->state->config.batchUpdate(
        converter.toXYZ(mission.flightboundary()),
        converter.toXYZ(mission.airdropboundary()),
        converter.toXYZ(mission.waypoints()),
        std::vector<Bottle>(mission.bottleassignments().begin(),
            mission.bottleassignments().end())
    );

    this->uploaded_mission = mission;


    const char* msg = "Mission uploaded";
    LOG_RESPONSE(INFO, msg, HTTPStatus::OK, msg, mime::plaintext);
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
    response.set_content(
        "TODO: calculate path using RRT, replace cached path, and return back!", "text/plain");
    response.status = HTTPStatus::NOT_IMPLEMENTED;
}

void GCSServer::_postPathInitialValidate(
    const httplib::Request& request, httplib::Response& response
) {
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
    response.set_content(
        "TODO: take a singular image with the camera and return as jpeg!", "text/plain");
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
