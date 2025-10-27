#include <google/protobuf/util/json_util.h>
#include <httplib.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>


#include <nlohmann/json.hpp>

#include "core/mission_state.hpp"
#include "network/gcs_macros.hpp"
#include "network/mavlink.hpp"
#include "pathing/mission_path.hpp"
#include "protos/obc.pb.h"
#include "ticks/airdrop_approach.hpp"
#include "ticks/cv_loiter.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/path_validate.hpp"
#include "ticks/tick.hpp"
#include "ticks/wait_for_takeoff.hpp"
#include "utilities/http.hpp"
#include "utilities/logging.hpp"
#include "utilities/serialize.hpp"

extern "C" {
#include "udp_squared/protocol.h"
}

using namespace std::chrono_literals;  // NOLINT
using json = nlohmann::json;

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

    std::list<std::pair<AirdropType, std::chrono::milliseconds>> lost_airdrop_conns;
    if (state->getAirdrop() == nullptr) {
        lost_airdrop_conns.push_back({AirdropType::Water, 99999ms});
        lost_airdrop_conns.push_back({AirdropType::Beacon, 99999ms});
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

    bool camera_good = false;

    if (state->getCamera()) {
        camera_good = state->getCamera()->isConnected();
    }

    OBCConnInfo info;
    for (auto const& [airdrop_index, ms_since_last_heartbeat] : lost_airdrop_conns) {
        info.add_dropped_airdrop_idx(airdrop_index);
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

        airdrop_t airdrop;
        if (airdrop_target.index() == AirdropType::Water) {
            airdrop = UDP2_A;
        } else if (airdrop_target.index() == AirdropType::Beacon) {
            airdrop = UDP2_B;
        } else {
            LOG_RESPONSE(ERROR, "Invalid bottle index", BAD_REQUEST);
            return;
        }

        float drop_lat = airdrop_target.coordinate().latitude();
        float drop_lng = airdrop_target.coordinate().longitude();
        state->getAirdrop()->send(makeLatLngPacket(SEND_LATLNG, airdrop, TARGET_ACQUIRED, drop_lat,
                                                   drop_lng, curr_alt_m));
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

    // START COMPRESSION
    // Compress the image before converting to base64
    std::vector<uchar> compressed_data;
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(85);  // Quality: 0-100, 85 is a good balance for transmission

    cv::Mat compressed_image;
    if (cv::imencode(".jpg", image->DATA, compressed_data, compression_params)) {
        // Create compressed Mat from encoded data
        compressed_image = cv::imdecode(compressed_data, cv::IMREAD_COLOR);

        if (!compressed_image.empty()) {
            LOG_F(INFO,
                  "Compressed manual capture image from %d bytes to %d bytes (%.1f%% compression)",
                  image->DATA.total() * image->DATA.elemSize(), compressed_data.size(),
                  (1.0 - static_cast<double>(compressed_data.size()) /
                             (image->DATA.total() * image->DATA.elemSize())) *
                      100.0);
        } else {
            LOG_F(WARNING, "Failed to decode compressed manual capture image, using original");
            compressed_image = image->DATA;
        }
    } else {
        LOG_F(WARNING, "Failed to compress manual capture image, using original");
        compressed_image = image->DATA;
    }

    ManualImage manual_image;
    manual_image.set_img_b64(cvMatToBase64(compressed_image));
    // END COMPRESSION

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
    LOG_F(INFO, "Received signal to drop current bottle ");

    if (state->getMav() == nullptr) {
        LOG_RESPONSE(ERROR, "Mavlink not connected", BAD_REQUEST);
        return;
    }

    // Copied from the integration test
    std::optional<airdrop_t> next_airdrop_to_drop;
    AirdropType next_airdrop = static_cast<AirdropType>(2);
    next_airdrop_to_drop = static_cast<airdrop_t>(next_airdrop);

    // std::string message;
    // drop
    if (triggerAirdrop(state->getMav(), next_airdrop_to_drop.value())) {
        LOG_RESPONSE(INFO, "Dropped Bottle Successfully", OK);
    } else {
        LOG_RESPONSE(INFO, "Failed to drop bottle", INTERNAL_SERVER_ERROR);
    }
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

DEF_GCS_HANDLE(Get, targets, all) {
    LOG_REQUEST("GET", "/targets/all");
    auto aggregator = state->getCV();
    if (!aggregator) {
        LOG_RESPONSE(ERROR, "CV not connected yet", BAD_REQUEST);
        return;
    }

    // 1) Lock aggregator & pop all new runs
    std::vector<AggregatedRun> new_runs = aggregator->popAllRuns();
    if (new_runs.empty()) {
        // No new data
        LOG_RESPONSE(INFO, "No new runs found", OK, "[]", mime::json);
        return;
    }

    // 2) Convert each AggregatedRun into ONE IdentifiedTarget proto
    std::vector<IdentifiedTarget> out_data;
    out_data.reserve(new_runs.size());  // Reserve space for efficiency

    //get aggregate to store a record of these results 
    LockPtr<std::map<int, IdentifiedTarget>> records = aggregator->getCVRecord();
    std::shared_ptr<std::map<int, IdentifiedTarget>> records_ptr = records.data;

    for (const auto& run : new_runs) {
        // Create ONE IdentifiedTarget message per AggregatedRun
        IdentifiedTarget target;
        // Set the run ID
        target.set_run_id(run.run_id);
        // START COMPRESSION

        // Compress the annotated image before converting to base64
        std::vector<uchar> compressed_data;
        std::vector<int> compression_params;
        compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
        compression_params.push_back(85);
        // Quality: 0-100, 85 is a good balance for transmission

        cv::Mat compressed_image;
        if (cv::imencode(".jpg", run.annotatedImage, compressed_data, compression_params)) {
            // Create compressed Mat from encoded data
            compressed_image = cv::imdecode(compressed_data, cv::IMREAD_COLOR);

            if (!compressed_image.empty()) {
                LOG_F(INFO, "Compressed image from %d bytes to %d bytes (%.1f%% compression)",
                      run.annotatedImage.total() * run.annotatedImage.elemSize(),
                      compressed_data.size(),
                      (1.0 - static_cast<double>(compressed_data.size()) /
                                 (run.annotatedImage.total() * run.annotatedImage.elemSize())) *
                          100.0);
            } else {
                LOG_F(WARNING, "Failed to decode compressed image, using original");
                compressed_image = run.annotatedImage;
            }
        } else {
            LOG_F(WARNING, "Failed to compress image, using original");
            compressed_image = run.annotatedImage;
        }

        // Convert the compressed image to base64 and set it (once per run)
        std::string b64 = cvMatToBase64(compressed_image);
        target.set_picture(b64);
        // END COMPRESSION

        // Ensure coords and bboxes vectors are the same size (should be guaranteed by Aggregator
        // logic)
        if (run.coords.size() != run.bboxes.size()) {
            LOG_F(ERROR,
                  "Mismatch between coordinates (%ld) and bboxes (%ld) count in run_id %d. "
                  "Skipping this run.",
                  run.coords.size(), run.bboxes.size(), run.run_id);
            continue;  // Skip this problematic run
        }

        // Add all coordinates and bounding boxes from this run
        for (size_t i = 0; i < run.bboxes.size(); ++i) {
            // Add coordinate
            GPSCoord* proto_coord = target.add_coordinates();  // Use the plural field name
            proto_coord->set_latitude(run.coords[i].latitude());
            proto_coord->set_longitude(run.coords[i].longitude());
            proto_coord->set_altitude(run.coords[i].altitude());

            // Add bounding box
            BboxProto* proto_bbox = target.add_bboxes();  // Use the plural field name
            proto_bbox->set_x1(run.bboxes[i].x1);
            proto_bbox->set_y1(run.bboxes[i].y1);
            proto_bbox->set_x2(run.bboxes[i].x2);
            proto_bbox->set_y2(run.bboxes[i].y2);
        }

        //copy the target to a record object to store
        IdentifiedTarget record;
        record.CopyFrom(target);
        //remove image
        record.set_picture("");
        records_ptr->insert_or_assign(run.run_id, target);
        
        // Add the completed IdentifiedTarget (representing the whole run) to the output list
        out_data.push_back(std::move(target));
    }  // End loop over AggregatedRuns
    // 3) Serialize the vector of IdentifiedTarget messages to JSON
    // Ensure messagesToJson can handle a vector or use iterators correctly
    std::string out_data_json = messagesToJson(out_data.begin(), out_data.end());
    response.set_content(out_data_json.c_str(), mime::json);
    response.status = OK;
}

// *** NEW Handler for POST /targets/matched ***
DEF_GCS_HANDLE(Post, targets, matched) {
    LOG_REQUEST("POST", "/targets/matched");

    if (state->getCV() == nullptr) {
        LOG_RESPONSE(ERROR, "CV not init yet", BAD_REQUEST);
        return;
    }

    nlohmann::json j_root = nlohmann::json::parse(request.body);

    LOG_S(INFO) << j_root;

    LockPtr<MatchedResults> matched_results = state->getCV()->getMatchedResults();

    if (matched_results.data == nullptr) {
        LOG_S(ERROR) << "lockptr is null";
    }

    AirdropTarget returned_matched_result;

    for (const auto& instance : j_root) {
        LOG_S(INFO) << returned_matched_result.index();
        google::protobuf::util::JsonStringToMessage(instance.dump(), &returned_matched_result);
        LOG_S(WARNING) << returned_matched_result.index();
        matched_results.data->matched_airdrop[returned_matched_result.index()] =
            returned_matched_result;
        LOG_S(ERROR) << returned_matched_result.index();
    }

    auto lock_ptr = state->getTickLockPtr<CVLoiterTick>();
    if (!lock_ptr.has_value()) {
        LOG_RESPONSE(WARNING, "Not currently in Loiter Tick", BAD_REQUEST);
        return;
    }
    lock_ptr->data->setStatus(CVLoiterTick::Status::Validated);

    LOG_RESPONSE(INFO, "Finished setting targets (and thus loitering)", OK);
}

DEF_GCS_HANDLE(Post, kill, kill, kill) {
    LOG_REQUEST("POST", "/kill/kill/kill");

    if (state->getMav() != nullptr) {
        state->getMav()->KILL_THE_PLANE_DO_NOT_CALL_THIS_ACCIDENTALLY();
        LOG_RESPONSE(ERROR, "Attempted to kill the plane", OK);
    } else {
        LOG_RESPONSE(ERROR, "Cannot kill the plane: no mav connection", BAD_REQUEST);
    }
}

DEF_GCS_HANDLE(Post, camera, startstream) {
    LOG_REQUEST("POST", "/camera/startstream");

    std::shared_ptr<CameraInterface> cam = state->getCamera();
    // const string
    uint32_t interval;
    unsigned long parsed_ul = std::stoul(request.body);  // NOLINT
    interval = static_cast<uint32_t>(parsed_ul);
    std::chrono::milliseconds chrono_interval(interval);

    if (!cam->isConnected()) {
        LOG_F(INFO, "Camera not connected. Attempting to connect...");
        cam->connect();
        if (!cam->isConnected()) {
            LOG_F(ERROR, "Failed to connect to the camera after connection attempt.");
            LOG_RESPONSE(ERROR, "Failed to connect to camera", NOT_FOUND);
            return;
        }
        LOG_F(INFO, "Camera connected successfully.");
    }

    cam->startStreaming();
    cam->startTakingPictures(chrono_interval, state->getMav());

    LOG_RESPONSE(INFO, "Started Camera Stream", OK);
}

DEF_GCS_HANDLE(Post, camera, endstream) {
    LOG_REQUEST("POST", "/camera/endstream");

    std::shared_ptr<CameraInterface> cam = state->getCamera();

    if (!cam->isConnected()) {
        LOG_F(INFO, "Camera not connected. Attempting to connect...");
        cam->connect();
        if (!cam->isConnected()) {
            LOG_F(ERROR, "Failed to connect to the camera after connection attempt.");
            LOG_RESPONSE(ERROR, "Failed to connect to camera", NOT_FOUND);
            return;
        }
        LOG_F(INFO, "Camera connected successfully.");
    }

    cam->stopTakingPictures();

    std::deque<ImageData> images;
    images = cam->getAllImages();
    for (const ImageData& image : images) {
        std::filesystem::path save_dir = state->config.camera.save_dir;
        std::filesystem::path img_filepath =
            save_dir / (std::to_string(image.TIMESTAMP) + std::string(".jpg"));
        std::filesystem::path json_filepath =
            save_dir / (std::to_string(image.TIMESTAMP) + std::string(".json"));
        saveImageToFile(image.DATA, img_filepath);
        if (image.TELEMETRY.has_value()) {
            saveImageTelemetryToFile(image.TELEMETRY.value(), json_filepath);
        }
        LOG_F(INFO, "Saving image %s", img_filepath.string().c_str());
    }

    LOG_RESPONSE(INFO, "Ended Camera Stream", OK);
}

DEF_GCS_HANDLE(Get, tickstate) {
    // Not using the macros here so that it doesn't scream at you every 1 second
    // LOG_REQUEST("GET", "/tickstate");

    TickID tickID = state->getTickID();
    std::string tick_state = TICK_ID_TO_STR(tickID);

    // LOG_RESPONSE(INFO, "Returning tick state", OK, tick_state, mime::plaintext);
    response.set_content(tick_state, mime::plaintext);
    response.status = OK;
}

DEF_GCS_HANDLE(Post, camera, runpipeline) {
    LOG_REQUEST("POST", "/camera/runpipeline");

    std::shared_ptr<CameraInterface> cam = state->getCamera();

    std::string yolo_model_dir = state->config.cv.yolo_model_dir;
    LOG_F(INFO, "Instantiating CV Aggregator with the following models:");
    LOG_F(INFO, "Yolo Model: %s", yolo_model_dir.c_str());

    // Make a CVAggregator instance and set it in the state
    state->setCV(std::make_shared<CVAggregator>(Pipeline(PipelineParams(yolo_model_dir))));

    if (!cam->isConnected()) {
        LOG_F(INFO, "Camera not connected. Attempting to connect...");
        cam->connect();
        if (!cam->isConnected()) {
            LOG_F(ERROR, "Failed to connect to the camera after connection attempt.");
            LOG_RESPONSE(ERROR, "Failed to connect to camera", NOT_FOUND);
            return;
        }
        LOG_F(INFO, "Camera connected successfully.");
    }

    cam->startStreaming();

    for (int i = 0; i < state->config.pathing.coverage.hover.pictures_per_stop; i++) {
        auto photo = state->getCamera()->takePicture(500ms, state->getMav());
        if (state->config.camera.save_images_to_file) {
            photo->saveToFile(state->config.camera.save_dir);
        }

        if (photo.has_value()) {
            // Run the pipeline on the photo
            state->getCV()->runPipeline(photo.value());
        }
    }

    LOG_RESPONSE(INFO, "Successfully ran camera Stream", OK);
}

DEF_GCS_HANDLE(Post, rtl) {
    LOG_REQUEST("POST", "/rtl");
    state->getMav()->rtl();
    LOG_RESPONSE(INFO, "RTL activated", OK);
}

// DEF_GCS_HANDLE(Get, oh, shit) {
//     LOG_REQUEST("GET", "/oh/shit");

//     if (state->getCV() == nullptr) {
//         LOG_RESPONSE(ERROR, "No CV yet :(", BAD_REQUEST);
//         return;
//     }

//     GPSCoord center;
//     center.set_latitude(38.31440741337194);
//     center.set_longitude(-76.54460728168489);
//     center.set_altitude(0);

//     LockPtr<CVResults> results = state->getCV()->getResults();

//     for (int i = 1; i <= 5; i++) {
//         CroppedTarget crop;
//         crop.isMannikin = false;
//         crop.croppedImage = cv::Mat(cv::Size(20, 20), CV_8UC3, cv::Scalar(255));

//         DetectedTarget target(center, static_cast<BottleDropIndex>(i), 100.0, crop);
//         target.coord = center;
//         target.likely_bottle = static_cast<BottleDropIndex>(i);
//         target.crop = crop;

//         results.data->detected_targets.push_back(target);
//     }

//     LOG_RESPONSE(INFO, "Oh shit", OK);
// }
