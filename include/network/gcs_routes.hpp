#ifndef INCLUDE_NETWORK_GCS_ROUTES_HPP_
#define INCLUDE_NETWORK_GCS_ROUTES_HPP_

#include <google/protobuf/util/json_util.h>
#include <httplib.h>

#include <vector>
#include <memory>
#include <string>
#include <optional>

#include <loguru.hpp>

#include "core/mission_state.hpp"
#include "protos/obc.pb.h"
#include "utilities/serialize.hpp"
#include "utilities/macros.hpp"
#include "ticks/tick.hpp"
#include "ticks/path_gen.hpp"

/*
 * This file defines all of the GCS handler functions for every route
 * the gcs server is listening on.
 * 
 * Inside of each of the functions, you have access to these variables
 * 
 * state: std::shared_ptr<MissionState>
 *        This is a shared pointer to the mission state, just like 
 *        other parts of the code have available.
 * request: const httplib::Request&
 *        Lets you access all of the information about the HTTP request
 *        itself.
 * response: httplib::Response&
 *        Lets you set all of the data to send back in the HTTP response.
 *        Note: you shouldn't need to access this for most things, as
 *        the LOG_RESPONSE macro will handle it for you.
 */

/*
 * GET /mission
 * ---
 * Response includes the information stored in the MissionConfig as a JSON object.
 * 
 * {
 *      TODO: fill in expected JSON output
 * }
 * 
 * 200 OK: no problems encountered
 */
DEF_GCS_HANDLE(Get, mission) {
    LOG_REQUEST("GET", "/mission");

    auto cached_mission = state->config.getCachedMission();
    if (cached_mission) {
        std::string output;
        google::protobuf::util::MessageToJsonString(*cached_mission, &output);

        LOG_RESPONSE(INFO, "Returning valid mission", OK, output.c_str(), mime::json);
    } else {
        LOG_RESPONSE(WARNING, "No mission uploaded", BAD_REQUEST);
    }
}

/*
 * POST /mission
 * 
 * {
 *      TODO: fill in the expected JSON format
 * }
 * 
 * TODO: reference protobuf class that encompasses the JSON
 * ---
 * Response is plain text that says whether posting was successful or not.
 * 200 OK: mission was in correct format and uploaded to server
 * 400 BAD REQUEST: mission was not in correct format; ignored
 */
DEF_GCS_HANDLE(Post, mission) {
    LOG_REQUEST("POST", "/mission");

    Mission mission;
    google::protobuf::util::JsonStringToMessage(request.body, &mission);

    // TODO: add checks for the mission

    // Update the cartesian converter to be centered around the new flight boundary
    state->setCartesianConverter(CartesianConverter(mission.flightboundary()));
    // Create the cartesian polygons for this new mission
    // and store in the mission state
    auto converter = state->getCartesianConverter().value();
    state->config.batchUpdate(
        converter.toXYZ(mission.flightboundary()),
        converter.toXYZ(mission.airdropboundary()),
        converter.toXYZ(mission.waypoints()),
        std::vector<Bottle>(mission.bottleassignments().begin(),
            mission.bottleassignments().end()),
        mission);

    LOG_RESPONSE(INFO, "Mission uploaded", OK);
}

/*
 * POST /airdrop
 * 
 * {
 *      TODO: fill in the expected JSON format
 * }
 * 
 * TODO: reference protobuf class that encompasses the JSON
 * ---
 * Response is plain text that says whether posting was successful or not
 * 200 OK: waypoints were in correct format and uploaded to server
 * 400 BAD REQUEST: waypoints were not in correct format; ignored
 */
DEF_GCS_HANDLE(Post, airdrop) {
    LOG_REQUEST("POST", "/airdrop");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

/*
 * GET /path/initial
 * 
 * TODO: reference protobuf class that encompasses the JSON
 * ---
 * Response is the cached initial path that hits all of the competition waypoints.
 * 
 * {
 *      TODO: fill in the expected JSON output
 * }
 * 
 * 200 OK: path was previously generated and returned
 * 404 NOT FOUND: no path has been generated yet
 * TODO: determine if there are more errors we might encounter
 */
DEF_GCS_HANDLE(Get, path, initial) {
    LOG_REQUEST("GET", "/path/initial");

    auto init_path = state->getInitPath();
    if (init_path.empty()) {
        LOG_RESPONSE(WARNING, "No initial path generated", BAD_REQUEST);
    } else {
        auto init_path = state->getInitPath();
        std::string json = messagesToJson(init_path.begin(), init_path.end());
        LOG_RESPONSE(INFO, "Got initial path", OK, json.c_str(), mime::json);
    }
}

/*
 * GET /path/initial/new
 * 
 * ---
 * JSON output is the same as _getPathInitial
 * 
 * This request explicitly requests a newly generated initial path. In contrast,
 * GET /path/initial requests a cached path. The cached path is automatically generated
 * when all of the mission config information has been received.
 */
DEF_GCS_HANDLE(Get, path, initial, new) {
    LOG_REQUEST("GET", "/path/initial/new");

    state->setTick(new PathGenerationTick(state));

    LOG_RESPONSE(INFO, "Started generating new initial path", OK);
}

/*
 * POST /path/initial/validate
 * 
 * ---
 * Specifies that the operator is happy with the current generated/cached initial path.
 * Progresses beyond the PathGenerationTick step by setting a "initial_path_validated"
 * flag in the Mission State
 * 
 * 200 OK: The initial path was generated, and is now validated 
 * 400 BAD REQUEST: There was no cached path to accept
 */
DEF_GCS_HANDLE(Post, path, initial, validate) {
    LOG_REQUEST("POST", "/path/initial/validate");

    if (state->getInitPath().empty()) {
        LOG_RESPONSE(WARNING, "No initial path generated", BAD_REQUEST);
    } else {
        state->validateInitPath();
        LOG_RESPONSE(INFO, "Initial path validated", OK);
    }
}

/*
 * GET /camera/status
 * ---
 * Response is a json object describing the status of the camera
 * 
 * {
 *      "connected": true/false,
 *      "streaming": true/false
 * } // TODO: verify that this JSON is not changing
 * 
 * 200 OK: camera status was successfuly captured
 * 500 INTERNAL SERVER ERROR: something went wrong with the camera. In this case,
 *         response will be in plain text format explaining what went wrong.
 */
DEF_GCS_HANDLE(Get, camera, status) {
    LOG_REQUEST("GET", "/camera/status");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

/*
 * POST /camera/start
 *        or
 * POST /camera/mock/start
 *        or
 * POST /camera/stop
 *        or
 * POST /camera/mock/stop
 * ---
 * Signifies that the camera/mock camera should start/stop taking images every X seconds. 
 * TODO: determine X, or allow it to be specified in the POST request.
 * 
 * Response is plain text describing the status of the camera.
 * 200 OK: Camera is now taking pictures/no longer taking pictures.
 * 500 INTERNAL SERVER ERROR: An error occurred.
 */
DEF_GCS_HANDLE(Post, camera, start) {
    LOG_REQUEST("POST", "/camera/start");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

DEF_GCS_HANDLE(Post, camera, stop) {
    LOG_REQUEST("POST", "/camera/stop");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

DEF_GCS_HANDLE(Post, camera, mock, start) {
    LOG_REQUEST("POST", "/camera/mock/start");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

DEF_GCS_HANDLE(Post, camera, mock, stop) {
    LOG_REQUEST("POST", "/camera/mock/stop");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}


/*
 * GET /camera/capture
 * ---
 * Signifies that the camera should take a picture. Sends back down the image
 * as a JPEG with the mimetype set correctly.
 */
DEF_GCS_HANDLE(Get, camera, capture) {
    LOG_REQUEST("GET", "/camera/capture");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

/*
 * GET /camera/config
 * ---
 * Requests the current configuration options for the camera.
 * 
 * {
 *      TODO: expected JSON output
 * }
 */
DEF_GCS_HANDLE(Get, camera, config) {
    LOG_REQUEST("GET", "/camera/config");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

/*
 * POST /camera/config
 * {
 *      TODO: expected JSON input
 * }
 * ---
 * Uploads the new configuration settings to use for the camera.
 */
DEF_GCS_HANDLE(Post, camera, config) {
    LOG_REQUEST("POST", "/camera/config");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

#endif  // INCLUDE_NETWORK_GCS_ROUTES_HPP_
