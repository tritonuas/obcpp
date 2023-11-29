#ifndef NETWORK_GCS_HPP
#define NETWORK_GCS_HPP

#include <thread>
#include <memory>
#include <mutex>
#include <cstdint>

#include <httplib.h>

#include "core/config.hpp"
#include "core/states.hpp"

enum HTTPStatus {
    OK = 200,

    BAD_REQUEST = 400,
    NOT_FOUND = 404,

    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
};

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
        void _bindHandlers(); // bind all the handlers to the server object

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
        void _getMission(const httplib::Request&, httplib::Response&);

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
        void _postMission(const httplib::Request&, httplib::Response&);

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
        void _postAirdropTargets(const httplib::Request&, httplib::Response&);

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
        void _getPathInitial(const httplib::Request&, httplib::Response&);

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
        void _getPathInitialNew(const httplib::Request&, httplib::Response&);

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
        void _postPathInitialValidate(const httplib::Request&, httplib::Response&);

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
        void _getCameraStatus(const httplib::Request&, httplib::Response&);

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
        void _postCameraStart(const httplib::Request&, httplib::Response&);
        void _postCameraStop(const httplib::Request&, httplib::Response&);
        void _postCameraMockStart(const httplib::Request&, httplib::Response&);
        void _postCameraMockStop(const httplib::Request&, httplib::Response&);

        /*
         * GET /camera/capture
         * ---
         * Signifies that the camera should take a picture. Sends back down the image
         * as a JPEG with the mimetype set correctly.
         */
        void _getCameraCapture(const httplib::Request&, httplib::Response&);

        /*
         * GET /camera/config
         * ---
         * Requests the current configuration options for the camera.
         * 
         * {
         *      TODO: expected JSON output
         * }
         */
        void _getCameraConfig(const httplib::Request&, httplib::Response&);

        /*
         * POST /camera/config
         * {
         *      TODO: expected JSON input
         * }
         * ---
         * Uploads the new configuration settings to use for the camera.
         */
        void _postCameraConfig(const httplib::Request&, httplib::Response&);
};

#endif // NETWORK_GCS_HPP