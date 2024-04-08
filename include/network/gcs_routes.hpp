#ifndef INCLUDE_NETWORK_GCS_ROUTES_HPP_
#define INCLUDE_NETWORK_GCS_ROUTES_HPP_

#include <google/protobuf/util/json_util.h>
#include <httplib.h>

#include <vector>
#include <memory>
#include <string>
#include <optional>

#include "utilities/logging.hpp"
#include "core/mission_state.hpp"
#include "protos/obc.pb.h"
#include "utilities/serialize.hpp"
#include "network/gcs_macros.hpp"
#include "ticks/tick.hpp"
#include "ticks/path_gen.hpp"

/*
 * GET /tick
 * ---
 * Returns the name of the current tick function, for display in the GCS control page
 * 
 * 200 OK: No problems occurred
 **/
DEF_GCS_HANDLE(Get, tick);

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
DEF_GCS_HANDLE(Get, mission);

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
DEF_GCS_HANDLE(Post, mission);

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
DEF_GCS_HANDLE(Post, airdrop);

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
DEF_GCS_HANDLE(Get, path, initial);

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
DEF_GCS_HANDLE(Get, path, initial, new);

/*
 * POST /path/initial/validate
 * 
 * ---
 * Specifies that the operator is happy with the current generated/cached initial path.
 * Progresses beyond the PathGenTick step by setting a "initial_path_validated"
 * flag in the Mission State
 * 
 * 200 OK: The initial path was generated, and is now validated 
 * 400 BAD REQUEST: There was no cached path to accept
 */
DEF_GCS_HANDLE(Post, path, initial, validate);
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
DEF_GCS_HANDLE(Get, camera, status);

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
DEF_GCS_HANDLE(Post, camera, start);
DEF_GCS_HANDLE(Post, camera, stop);
DEF_GCS_HANDLE(Post, camera, mock, start);
DEF_GCS_HANDLE(Post, camera, mock, stop);

/*
 * GET /camera/capture
 * ---
 * Signifies that the camera should take a picture. Sends back down the image
 * as a JPEG with the mimetype set correctly.
 */
DEF_GCS_HANDLE(Get, camera, capture);

/*
 * GET /camera/config
 * ---
 * Requests the current configuration options for the camera.
 * 
 * {
 *      TODO: expected JSON output
 * }
 */
DEF_GCS_HANDLE(Get, camera, config);

/*
 * POST /camera/config
 * {
 *      TODO: expected JSON input
 * }
 * ---
 * Uploads the new configuration settings to use for the camera.
 */
DEF_GCS_HANDLE(Post, camera, config);

#endif  // INCLUDE_NETWORK_GCS_ROUTES_HPP_
