#ifndef INCLUDE_NETWORK_GCS_ROUTES_HPP_
#define INCLUDE_NETWORK_GCS_ROUTES_HPP_

#include <google/protobuf/util/json_util.h>
#include <httplib.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "core/mission_state.hpp"
#include "network/gcs_macros.hpp"
#include "protos/obc.pb.h"
#include "ticks/path_gen.hpp"
#include "ticks/tick.hpp"
#include "utilities/logging.hpp"
#include "utilities/serialize.hpp"

/*
 * GET /connection
 * ---
 * Returns information about the connection status of the OBC
 *
 * 200 OK: Successfully retrieved data
 */
DEF_GCS_HANDLE(Get, connections);

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
 * POST /targets/locations
 * ---
 * Upload the bottle locations to the OBC
 */
DEF_GCS_HANDLE(Post, targets, locations);

/*
 * GET /path/initial
 *
 * 200 OK: path was previously generated and returned
 * 404 NOT FOUND: no path has been generated yet
 * TODO: determine if there are more errors we might encounter
 */
DEF_GCS_HANDLE(Get, path, initial);

/*
 * GET /path/coverage
 *
 * 200 OK: path was previously generated and returned
 * 404 NOT FOUND: no path has been generated yet
 * TODO: determine if there are more errors we might encounter
 */
DEF_GCS_HANDLE(Get, path, coverage);

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
 * GET /camera/capture
 * ---
 * Signifies that the camera should take a picture. Sends back down the image
 * as a JPEG with the mimetype set correctly.
 */
DEF_GCS_HANDLE(Get, camera, capture);

/**
 * POST /dodropnow
 * ---
 * Tells the OBC to do a drop now, for the given bottle
 * Expects the body to be a BottleSwap proto message in json format
 */
DEF_GCS_HANDLE(Post, dodropnow);

/**
 * POST /takeoff/manual
 * ---
 */
DEF_GCS_HANDLE(Post, takeoff, manual);

/**
 * POST /takeoff/autonomous
 * ---
 */
DEF_GCS_HANDLE(Post, takeoff, autonomous);

/**
 * POST /targets/validate
 * ---
 * validate the current set of matched targets and go onto airdrop mode
 */
DEF_GCS_HANDLE(Post, targets, validate);

/**
 * POST /targets/reject
 * ---
 * tell the plane to do the search path again because we missed targets
 */
DEF_GCS_HANDLE(Post, targets, reject);

/**
 * GET /targets/all
 * ---
 * get all of the targets that have been identified & send to the GCS
 */
DEF_GCS_HANDLE(Get, targets, all);

/**
 * GET /targets/matched
 * ---
 * get the current mappings between identified targets and bottles
 */
DEF_GCS_HANDLE(Get, targets, matched);

/**
 * POST /targets/matched
 * ---
 * manual override for mappings between targets
 */
DEF_GCS_HANDLE(Post, targets, matched);

/**
 * POST /kill/kill/kill
 *
 * TELLS THE PLANE TO CRASH ITSELF
 *
 * ONLY CALL THIS IN EMERGENCY SITUATIONS
 */
DEF_GCS_HANDLE(Post, kill, kill, kill);

/**
 * POST /camera/startstream
 * ---
 * Testing function to start the stream at a specific interval
 * Expects a uint32 integer in the POST body as the interval to call startTakingPictures()
 * 
 */
DEF_GCS_HANDLE(Post, camera, startstream);

/**
 * POST /camera/endstream
 * ---
 * Testing function to end the stream. 
 * Takes the pictures in the camera queue and stores it in the config file path
 */
DEF_GCS_HANDLE(Post, camera, endstream);

DEF_GCS_HANDLE(Post, rtl);

DEF_GCS_HANDLE(Get, oh, shit);

DEF_GCS_HANDLE(Get, tickstate);
DEF_GCS_HANDLE(Post, camera, runpipeline);


#endif  // INCLUDE_NETWORK_GCS_ROUTES_HPP_
