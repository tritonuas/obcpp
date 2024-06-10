#include <google/protobuf/util/json_util.h>
#include <httplib.h>

#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <filesystem>

#include "core/mission_state.hpp"
#include "protos/obc.pb.h"
#include "utilities/serialize.hpp"
#include "utilities/logging.hpp"
#include "utilities/http.hpp"
#include "network/gcs_macros.hpp"
#include "ticks/tick.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/path_validate.hpp"
#include "ticks/wait_for_takeoff.hpp"

using namespace std::chrono_literals; // NOLINT

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
    LOG_REQUEST_TRACE("GET", "/connections");

    std::list<std::pair<BottleDropIndex, std::chrono::milliseconds>> lost_airdrop_conns;
    if (state->getAirdrop() == nullptr) {
        lost_airdrop_conns.push_back({BottleDropIndex::A, 99999ms});
        lost_airdrop_conns.push_back({BottleDropIndex::B, 99999ms});
        lost_airdrop_conns.push_back({BottleDropIndex::C, 99999ms});
        lost_airdrop_conns.push_back({BottleDropIndex::D, 99999ms});
        lost_airdrop_conns.push_back({BottleDropIndex::E, 99999ms});
    } else {
        lost_airdrop_conns = state->getAirdrop()->getLostConnections(3s);
    }

    mavsdk::Telemetry::RcStatus mav_conn;

    if (state->getMav() == nullptr) {
        mav_conn.is_available = false;
        mav_conn.signal_strength_percent = 0.0;
    } else {
        mav_conn = state->getMav()->get_conn_status();
    }

    bool camera_good = state->getCamera()->isConnected();

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

    LOG_RESPONSE(TRACE, "Returning conn info", OK, output.c_str(), mime::json);
}

DEF_GCS_HANDLE(Get, tick) {
    LOG_REQUEST("GET", "/tick");

    LOG_RESPONSE(INFO, TICK_ID_TO_STR(state->getTickID()), OK);
}

DEF_GCS_HANDLE(Get, mission) {
    LOG_REQUEST("GET", "/mission");

    auto cached_mission = state->mission_params.getCachedMission();
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

    auto err = state->mission_params.setMission(mission, state->getCartesianConverter().value());
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
    lock_ptr->data->setStatus(PathValidateTick::Status::Rejected);

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
    lock_ptr->data->setStatus(PathValidateTick::Status::Validated);

    LOG_RESPONSE(INFO, "Initial path validated", OK);
}

DEF_GCS_HANDLE(Get, camera, capture) {
    LOG_REQUEST("GET", "/camera/capture");

    std::shared_ptr<CameraInterface> cam = state->getCamera();

    if (!cam->isConnected()) {
        cam->connect();
    }

    cam->startStreaming();

    std::optional<ImageData> image = cam->takePicture(1000ms, state->getMav());

    if (!image.has_value()) {
        LOG_RESPONSE(ERROR, "Failed to capture image", INTERNAL_SERVER_ERROR);
        return;
    }

    std::optional<ImageTelemetry> telemetry = image->TELEMETRY;

    try {
        std::filesystem::path save_dir = state->camera_config.save_dir;
        std::filesystem::path img_filepath = save_dir / (std::to_string(image->TIMESTAMP) + std::string(".jpg")); //NOLINT
        std::filesystem::path json_filepath = save_dir / (std::to_string(image->TIMESTAMP) + std::string(".json")); //NOLINT
        saveImageToFile(image->DATA, img_filepath);
        if (image->TELEMETRY.has_value()) {
            saveImageTelemetryToFile(image->TELEMETRY.value(), json_filepath);
        }
    } catch (std::exception& e) {
        LOG_F(ERROR, "Failed to save image and telemetry to file");
    }

    ManualImage manual_image;
    manual_image.set_img_b64(cvMatToBase64(image->DATA));
    manual_image.set_timestamp(image->TIMESTAMP);
    if (telemetry.has_value()) {
        manual_image.set_lat_deg(telemetry->latitude_deg);
        manual_image.set_lng_deg(telemetry->longitude_deg);
        manual_image.set_alt_agl_m(telemetry->altitude_agl_m);
        manual_image.set_airspeed_m_s(telemetry->airspeed_m_s);
        manual_image.set_heading_deg(telemetry->heading_deg);
        manual_image.set_yaw_deg(telemetry->yaw_deg);
        manual_image.set_pitch_deg(telemetry->pitch_deg);
        manual_image.set_roll_deg(telemetry->roll_deg);
    }

    std::string output;
    google::protobuf::util::MessageToJsonString(manual_image, &output);

    LOG_RESPONSE(INFO, "Successfully captured image", OK, output.c_str(), mime::json);
}

DEF_GCS_HANDLE(Post, dodropnow) {
    LOG_REQUEST("POST", "/dodropnow");

    BottleSwap bottle_proto;
    google::protobuf::util::JsonStringToMessage(request.body, &bottle_proto);

    bottle_t bottle;
    if (bottle_proto.index() == BottleDropIndex::A) {
        bottle = UDP2_A;
    } else if (bottle_proto.index() == BottleDropIndex::B) {
        bottle = UDP2_B;
    } else if (bottle_proto.index() == BottleDropIndex::C) {
        bottle = UDP2_C;
    } else if (bottle_proto.index() == BottleDropIndex::D) {
        bottle = UDP2_D;
    } else if (bottle_proto.index() == BottleDropIndex::E) {
        bottle = UDP2_E;
    } else {
        LOG_RESPONSE(ERROR, "Invalid bottle index", BAD_REQUEST);
        return;
    }

    LOG_F(INFO, "Received signal to drop bottle %d", bottle);

    if (state->getAirdrop() == nullptr) {
        LOG_RESPONSE(ERROR, "Airdrop not connected", BAD_REQUEST);
        return;
    }

    state->getAirdrop()->send(makeDropNowPacket(bottle));

    LOG_RESPONSE(INFO, "Dropped bottle", OK);
}

DEF_GCS_HANDLE(Post, takeoff, manual) {
    LOG_REQUEST("POST", "takeoff/manual");

    auto lock_ptr = state->getTickLockPtr<WaitForTakeoffTick>();
    if (!lock_ptr.has_value()) {
        LOG_RESPONSE(WARNING, "Not currently in WaitForTakeoff Tick", BAD_REQUEST);
        return;
    }
    lock_ptr->data->setStatus(WaitForTakeoffTick::Status::Manual);
    LOG_RESPONSE(INFO, "Set status of WaitForTakeoff Tick to manaul", OK);
}

DEF_GCS_HANDLE(Post, takeoff, autonomous) {
    LOG_REQUEST("POST", "takeoff/autonomous");

    auto lock_ptr = state->getTickLockPtr<WaitForTakeoffTick>();
    if (!lock_ptr.has_value()) {
        LOG_RESPONSE(WARNING, "Not currently in WaitForTakeoff Tick", BAD_REQUEST);
        return;
    }
    lock_ptr->data->setStatus(WaitForTakeoffTick::Status::Autonomous);
    LOG_RESPONSE(INFO, "Set status of WaitForTakeoff Tick to autonomous", OK);
}
