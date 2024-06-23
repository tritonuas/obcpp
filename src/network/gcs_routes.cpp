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
#include "pathing/mission_path.hpp"
#include "network/gcs_macros.hpp"
#include "ticks/tick.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/path_validate.hpp"
#include "ticks/wait_for_takeoff.hpp"
#include "ticks/cv_loiter.hpp"

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

DEF_GCS_HANDLE(Post, targets, locations) {
    LOG_REQUEST("POST", "/targets/locations");

    if (state->getAirdrop() == nullptr) {
        LOG_RESPONSE(ERROR, "Airdrop not connected", BAD_REQUEST);
        return;
    }

    uint32_t curr_alt_m;

    if (state->getMav() == nullptr) {
        LOG_RESPONSE(ERROR, "Mavlink not connected", BAD_REQUEST);
        return;
    } else {
        curr_alt_m = state->getMav()->altitude_msl_m();
    }

    nlohmann::json waypoints = nlohmann::json::parse(request.body);
    AirdropTarget airdrop_target;

    if (!waypoints.is_array()) {
        LOG_RESPONSE(ERROR, "Waypoints is not a vactor", BAD_REQUEST);
    }

    for (const auto& waypoint : waypoints) {
        google::protobuf::util::JsonStringToMessage(waypoint.dump(), &airdrop_target);

        bottle_t bottle;
        if (airdrop_target.index() == BottleDropIndex::A) {
            bottle = UDP2_A;
        } else if (airdrop_target.index() == BottleDropIndex::B) {
            bottle = UDP2_B;
        } else if (airdrop_target.index() == BottleDropIndex::C) {
            bottle = UDP2_C;
        } else if (airdrop_target.index() == BottleDropIndex::D) {
            bottle = UDP2_D;
        } else if (airdrop_target.index() == BottleDropIndex::E) {
            bottle = UDP2_E;
        } else {
            LOG_RESPONSE(ERROR, "Invalid bottle index", BAD_REQUEST);
            return;
        }

        float drop_lat = airdrop_target.coordinate().latitude();
        float drop_lng = airdrop_target.coordinate().longitude();
        state->getAirdrop()->send(
            makeLatLngPacket(SEND_LATLNG, bottle, TARGET_ACQUIRED, drop_lat, drop_lng, curr_alt_m));
    }
    LOG_RESPONSE(INFO, "Uploaded airdrop targets coordinates", OK);
}

DEF_GCS_HANDLE(Get, path, initial) {
    LOG_REQUEST("GET", "/path/initial");

    auto path = state->getInitPath();
    if (path.get().empty()) {
        LOG_RESPONSE(WARNING, "No initial path generated", BAD_REQUEST);
    } else {
        std::string json = messagesToJson(path.get().begin(), path.get().end());
        LOG_RESPONSE(INFO, "Got initial path", OK, json.c_str(), mime::json);
    }
}

DEF_GCS_HANDLE(Get, path, coverage) {
    LOG_REQUEST("GET", "/path/coverage");

    auto path = state->getCoveragePath();
    if (path.get().empty()) {
        LOG_RESPONSE(WARNING, "No coverage path generated", BAD_REQUEST);
    } else {
        std::string json = messagesToJson(path.get().begin(), path.get().end());
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

    if (state->getInitPath().get().empty()) {
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

    if (state->config.camera.save_images_to_file) {
        image->saveToFile(state->config.camera.save_dir);
    }

    if (!image.has_value()) {
        LOG_RESPONSE(ERROR, "Failed to capture image", INTERNAL_SERVER_ERROR);
        return;
    }

    std::optional<ImageTelemetry> telemetry = image->TELEMETRY;

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
    LOG_REQUEST("POST", "/takeoff/manual");

    auto lock_ptr = state->getTickLockPtr<WaitForTakeoffTick>();
    if (!lock_ptr.has_value()) {
        LOG_RESPONSE(WARNING, "Not currently in WaitForTakeoff Tick", BAD_REQUEST);
        return;
    }
    lock_ptr->data->setStatus(WaitForTakeoffTick::Status::Manual);
    LOG_RESPONSE(INFO, "Set status of WaitForTakeoff Tick to manaul", OK);
}

DEF_GCS_HANDLE(Post, takeoff, autonomous) {
    LOG_REQUEST("POST", "/takeoff/autonomous");

    auto lock_ptr = state->getTickLockPtr<WaitForTakeoffTick>();
    if (!lock_ptr.has_value()) {
        LOG_RESPONSE(WARNING, "Not currently in WaitForTakeoff Tick", BAD_REQUEST);
        return;
    }
    lock_ptr->data->setStatus(WaitForTakeoffTick::Status::Autonomous);
    LOG_RESPONSE(INFO, "Set status of WaitForTakeoff Tick to autonomous", OK);
}

DEF_GCS_HANDLE(Post, targets, validate) {
    LOG_REQUEST("POST", "/targets/validate");
    auto lock_ptr = state->getTickLockPtr<CVLoiterTick>();

    if (!lock_ptr.has_value()) {
        LOG_RESPONSE(WARNING, "Not currently in CVLoiter Tick", BAD_REQUEST);
        return;
    }

    lock_ptr->data->setStatus(CVLoiterTick::Status::Validated);
    LOG_RESPONSE(INFO, "Set status of CVLoiter Tick to validated", OK);
}

DEF_GCS_HANDLE(Post, targets, reject) {
    LOG_REQUEST("POST", "/targets/reject");
    auto lock_ptr = state->getTickLockPtr<CVLoiterTick>();

    if (!lock_ptr.has_value()) {
        LOG_RESPONSE(WARNING, "Not currently in CVLoiter Tick", BAD_REQUEST);
        return;
    }

    lock_ptr->data->setStatus(CVLoiterTick::Status::Rejected);
    LOG_RESPONSE(INFO, "Set status of CVLoiter Tick to rejected", OK);
}

DEF_GCS_HANDLE(Get, targets, all) {
    LOG_REQUEST("GET", "/targets/all");

    LockPtr<CVResults> results = state->getCV()->getResults();

    // Convert to protobuf serialization
    std::vector<IdentifiedTarget> out_data;

    int id = 0; // id of the target is the index in the detected_targets vector
    // See layout of Identified target proto here:
    // https://github.com/tritonuas/protos/blob/master/obc.proto
    for (auto& target : results.data->detected_targets) {
        IdentifiedTarget out;
        out.set_id(id);
        out.set_picture(cvMatToBase64(target.crop.croppedImage));
        out.set_allocated_coordinate(&target.coord);
        out.set_ismannikin(target.crop.isMannikin);

        // not setting target info because that doesn't really make sense with the current context
        // of the matching algorithm, since the algorithm is matching cropped targets to the
        // expected not-stolen generated images, so it isn't really classifying targets with a 
        // specific alphanumeric, color, etc...

        id++;
    }

    std::string out_data_json = messagesToJson(out_data.begin(), out_data.end());
    LOG_RESPONSE(INFO, "Got serialized target data", OK, out_data_json.c_str(), mime::json);
}

DEF_GCS_HANDLE(Get, targets, matched) {
    LOG_REQUEST("GET", "/targets/matched");

    /*
        NOTE / TODO:
        ok so these protobuf messages should really be refactored a bit
        currently the MatchedTarget protobuf message contains both a Bottle and
        IdentifiedTarget, which in theory seems fine but gets annoying because
        now to make the message to send down here we have to construct an entire IdentifiedTarget
        struct and then send that down, when really we should only need to send the id
        down because all of the IdentifiedTarget information should have been sent
        in the GET /targets/all endpoint
        
        That is what this stupid vector is. We have to construct a bunch of Identified targets
        so that we can send them down, but because of the way protobuf works we have to
        call the set_allocated_target function which takes a pointer to an IdentifiedTarget.
        For the bottles this is ok because we actually store the Bottles inside of mission
        parameters, so we can get a vector of all of the bottles easily and just use the
        pointer to that copied vector which we get below this vector. But for this one
        we actually need to allocate a bunch of identified targets such that they
        don't go out of scope before we use them for serialization. This vector will
        basically "own" the constructed identified targets we make so that they don't
        go out of scope until the end of this function.

        And this all could be solved by changing the MatchedTargets protobuf message
        to just take an index instead of an IdentifiedTarget, but I'm scared to do that
        two days before competition because idk if there are other assumptions on this
        protobuf message right now, especially in the frontend, and I don't want to rewrite
        the frontend Report page...

        - tyler
    */
    std::vector<std::unique_ptr<IdentifiedTarget>> alloc_targets;

    // this vector of bottles needs to live as long as this function because we need to give a ptr
    // to these bottles when constructing the MatchedTarget protobuf, and we don't want that data
    // to go out of scope and become garbage
    std::vector<Bottle> bottles = state->mission_params.getAirdropBottles();

    // convert to protobuf serialization
    std::vector<MatchedTarget> out_data;

    LockPtr<CVResults> results = state->getCV()->getResults();
    for (const auto& [bottle, target_index] : results.data->matches) {
        if (target_index >= results.data->detected_targets.size()) {
            LOG_RESPONSE(ERROR, "Out of bounds match error", INTERNAL_SERVER_ERROR);
            return;
        }

        const DetectedTarget& detected_target = results.data->detected_targets.at(target_index);

        MatchedTarget matched_target;
        
        Bottle* curr_bottle = nullptr;
        for (auto& b : bottles) {
            if (b.index() == target_index) {
                curr_bottle = &b;
            }
        }
        if (curr_bottle == nullptr) {
            LOG_RESPONSE(ERROR, "Cannot find bottle with right index", INTERNAL_SERVER_ERROR);
            return;
        }
        matched_target.set_allocated_bottle(curr_bottle);

        auto identified_target = std::make_unique<IdentifiedTarget>();

        identified_target->set_id(target_index);

        // don't think we need to set anything other than the ID ? (as per long comment up above)

        matched_target.set_allocated_target(identified_target.get());
        alloc_targets.push_back(std::move(identified_target)); // keep the ptr alive

        out_data.push_back(matched_target);
    }

    std::string out_data_json = messagesToJson(out_data.begin(), out_data.end());
    LOG_RESPONSE(INFO, "Got serialized target match data", OK, out_data_json.c_str(), mime::json);
}

DEF_GCS_HANDLE(Post, targets, matched) {
    LOG_REQUEST("POST", "/targets/matched");

    ManualTargetMatch matchings;
    google::protobuf::util::JsonStringToMessage(request.body, &matchings);

    LockPtr<CVResults> results = state->getCV()->getResults();

    // obviously this should be cleaned up, but it should work for now
    if (matchings.bottle_a_id() >= 0) { // negative is invalid
        LOG_F(INFO, "Updating bottle A to id %d", matchings.bottle_a_id());
        results.data->matches[BottleDropIndex::A] = static_cast<size_t>(matchings.bottle_a_id());
    }
    if (matchings.bottle_b_id() >= 0) {
        LOG_F(INFO, "Updating bottle B to id %d", matchings.bottle_b_id());
        results.data->matches[BottleDropIndex::B] = static_cast<size_t>(matchings.bottle_b_id());
    }
    if (matchings.bottle_c_id() >= 0) {
        LOG_F(INFO, "Updating bottle C to id %d", matchings.bottle_c_id());
        results.data->matches[BottleDropIndex::C] = static_cast<size_t>(matchings.bottle_c_id());
    }
    if (matchings.bottle_d_id() >= 0) {
        LOG_F(INFO, "Updating bottle D to id %d", matchings.bottle_d_id());
        results.data->matches[BottleDropIndex::D] = static_cast<size_t>(matchings.bottle_d_id());
    }
    if (matchings.bottle_e_id() >= 0) {
        LOG_F(INFO, "Updating bottle E to id %d", matchings.bottle_e_id());
        results.data->matches[BottleDropIndex::E] = static_cast<size_t>(matchings.bottle_e_id());
    }

    LOG_RESPONSE(INFO, "Updated bottle matchings", OK);
}
