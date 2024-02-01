#include "network/gcs.hpp"

#include <google/protobuf/util/json_util.h>
#include <httplib.h>
#include <loguru.hpp>

#include <memory>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>

#include "core/config.hpp"
#include "core/states.hpp"
#include "core/ticks.hpp"
#include "utilities/locks.hpp"
#include "utilities/serialize.hpp"
#include "protos/obc.pb.h"
#include "pathing/cartesian.hpp"

// Should be called at the beginning of every handler function so we can
// log out all of the relevant information
#define LOG_REQUEST(method, route) \
    LOG_SCOPE_F(INFO, "%s %s", method, route); \
    LOG_F(INFO, "User-Agent: %s", request.get_header_value("User-Agent").c_str())

// One of the LOG_RESPONSE logging functions should be used to both log and
// set the HTTP response 
#define LOG_RESPONSE_5(LOG_LEVEL, msg, response_code, body, mime) \
    if (msg != body) LOG_F(LOG_LEVEL, "%s", msg); \
    LOG_F(LOG_LEVEL, "HTTP %d: %s", response_code, HTTP_STATUS_TO_STRING.at(response_code)); \
    LOG_F(LOG_LEVEL, "%s", body); \
    response.set_content(body, mime); \
    response.status = response_code

// Essentially a special case of the 5 param log function, where
// the message body is in plaintext and is also what you want to log
#define LOG_RESPONSE_3(LOG_LEVEL, msg, response_code) \
    LOG_RESPONSE_5(LOG_LEVEL, msg, response_code, msg, mime::plaintext)

// Mechanism to overload the 2 different logging macros
#define GET_MACRO(_1,_2,_3,_4,_5,NAME,...) NAME
#define LOG_RESPONSE(...) GET_MACRO(__VA_ARGS__, LOG_RESPONSE_5, _4, LOG_RESPONSE_3)(__VA_ARGS__)

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
    LOG_REQUEST("GET", "/mission");

    if (this->uploaded_mission) {
        std::string output;
        google::protobuf::util::MessageToJsonString(this->uploaded_mission.value(), &output);

        LOG_RESPONSE(INFO, "Returning valid mission", OK, output.c_str(), mime::json);
    } else {
        LOG_RESPONSE(WARNING, "No mission uploaded", BAD_REQUEST);
    }
}

void GCSServer::_postMission(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("POST", "/mission");
    
    Mission mission;
    google::protobuf::util::JsonStringToMessage(request.body, &mission);

    // TODO: add checks for the mission

    // Update the cartesian converter to be centered around the new flight boundary
    this->state->setCartesianConverter(CartesianConverter(mission.flightboundary()));
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

    LOG_RESPONSE(INFO, "Mission uploaded", OK);
}

void GCSServer::_postAirdropTargets(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("POST", "/airdrop");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

void GCSServer::_getPathInitial(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("GET", "/path/initial");

    auto init_path = this->state->getInitPath();
    if (init_path.empty()) {
        LOG_RESPONSE(WARNING, "No initial path generated", BAD_REQUEST);
    } else {
        auto init_path = this->state->getInitPath();
        std::string json = messagesToJson(init_path.begin(), init_path.end());
        LOG_RESPONSE(INFO, "Got initial path", OK, json.c_str(), mime::json);
    }
}

void GCSServer::_getPathInitialNew(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("GET", "/path/initial/new");

    this->state->setTick(new PathGenerationTick(this->state));

    LOG_RESPONSE(INFO, "Started generating new initial path", OK);
}

void GCSServer::_postPathInitialValidate(
    const httplib::Request& request, httplib::Response& response
) {
    LOG_REQUEST("POST", "/path/initial/validate");

    if (state->getInitPath().empty()) {
        LOG_RESPONSE(WARNING, "No initial path generated", BAD_REQUEST); 
    } else {
        this->state->validateInitPath();
        LOG_RESPONSE(INFO, "Initial path validated", OK);
    }
}

void GCSServer::_getCameraStatus(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("GET", "/camera/status");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

void GCSServer::_postCameraStart(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("POST", "/camera/start");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

void GCSServer::_postCameraMockStart(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("POST", "/camera/mock/start");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

void GCSServer::_postCameraStop(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("POST", "/camera/stop");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

void GCSServer::_postCameraMockStop(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("POST", "/camera/mock/stop");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

void GCSServer::_getCameraCapture(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("GET", "/camera/capture");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

void GCSServer::_getCameraConfig(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("GET", "/camera/config");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

void GCSServer::_postCameraConfig(const httplib::Request& request, httplib::Response& response) {
    LOG_REQUEST("POST", "/camera/config");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}
