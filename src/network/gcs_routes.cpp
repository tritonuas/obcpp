#include <google/protobuf/util/json_util.h>
#include <httplib.h>

#include <vector>
#include <memory>
#include <string>
#include <optional>

#include "core/mission_state.hpp"
#include "protos/obc.pb.h"
#include "utilities/serialize.hpp"
#include "utilities/http.hpp"
#include "utilities/logging.hpp"
#include "network/gcs_macros.hpp"
#include "ticks/tick.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/path_validate.hpp"

using namespace std::chrono_literals;

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

DEF_GCS_HANDLE(Get, connections) {
    LOG_REQUEST("GET", "/connections");

    auto lost_airdrop_conns = state->getAirdrop()->getLostConnections(3s);
    auto mav_conn = state->getMav()->get_conn_status();
    // TODO: query the camera status
    bool camera_good = false;

    OBCConnInfo info;
    for (auto const& [bottle_index, ms_since_last_heartbeat] : lost_airdrop_conns) {
        info.add_dropped_bottle_idx(bottle_index);
        info.add_ms_since_ad_heartbeat(ms_since_last_heartbeat.count());
    }
    info.set_mav_rc_good(mav_conn.is_available);
    info.set_mav_rc_strength(mav_conn.signal_strength_percent);
    info.set_camera_good(camera_good);

    std::string output;
    google::protobuf::util::MessageToJsonString(info, &output);

    LOG_RESPONSE(INFO, "Returning conn info", OK, output.c_str(), mime::json);
}

DEF_GCS_HANDLE(Get, tick) {
    LOG_REQUEST("GET", "/tick");

    LOG_RESPONSE(INFO, TICK_ID_TO_STR(state->getTickID()), OK);
}

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

DEF_GCS_HANDLE(Post, mission) {
    LOG_REQUEST("POST", "/mission");

    Mission mission;
    google::protobuf::util::JsonStringToMessage(request.body, &mission);

    // Update the cartesian converter to be centered around the new flight boundary
    state->setCartesianConverter(CartesianConverter(mission.flightboundary()));

    auto err = state->config.setMission(mission, state->getCartesianConverter().value());
    if (err.has_value()) {
        LOG_RESPONSE(WARNING, err.value().c_str(), BAD_REQUEST);
    } else {
        LOG_RESPONSE(INFO, "Mission uploaded", OK);
    }
}

DEF_GCS_HANDLE(Post, airdrop) {
    LOG_REQUEST("POST", "/airdrop");


    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

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

DEF_GCS_HANDLE(Get, path, initial, new) {
    LOG_REQUEST("GET", "/path/initial/new");

    auto lock_ptr = state->getTickLockPtr<PathValidateTick>();
    if (!lock_ptr.has_value()) {
        LOG_RESPONSE(WARNING, "Not currently in PathValidate Tick", BAD_REQUEST);
        return;
    }
    lock_ptr->ptr->setStatus(PathValidateTick::Status::Rejected);

    LOG_RESPONSE(INFO, "Started generating new initial path", OK);
}

DEF_GCS_HANDLE(Post, path, initial, validate) {
    LOG_REQUEST("POST", "/path/initial/validate");

    if (state->getInitPath().empty()) {
        LOG_RESPONSE(WARNING, "No initial path generated", BAD_REQUEST);
        return;
    }

    auto lock_ptr = state->getTickLockPtr<PathValidateTick>();
    if (!lock_ptr.has_value()) {
        LOG_RESPONSE(WARNING, "Not currently in PathValidate Tick", BAD_REQUEST);
        return;
    }
    lock_ptr->ptr->setStatus(PathValidateTick::Status::Validated);

    LOG_RESPONSE(INFO, "Initial path validated", OK);
}

DEF_GCS_HANDLE(Get, camera, status) {
    LOG_REQUEST("GET", "/camera/status");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

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

DEF_GCS_HANDLE(Get, camera, capture) {
    LOG_REQUEST("GET", "/camera/capture");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

DEF_GCS_HANDLE(Get, camera, config) {
    LOG_REQUEST("GET", "/camera/config");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}

DEF_GCS_HANDLE(Post, camera, config) {
    LOG_REQUEST("POST", "/camera/config");

    LOG_RESPONSE(WARNING, "Not Implemented", NOT_IMPLEMENTED);
}
