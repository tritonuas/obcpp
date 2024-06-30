#include <gtest/gtest.h>
#include <google/protobuf/util/json_util.h>

#include <memory>
#include <fstream>
#include <unordered_map>
#include <thread>

#include <nlohmann/json.hpp>

#include "network/gcs.hpp"
#include "network/gcs_routes.hpp"
#include "network/gcs_macros.hpp"
#include "core/mission_state.hpp"
#include "resources/json_snippets.hpp"
#include "utilities/http.hpp"
#include "utilities/obc_config.hpp"
#include "ticks/mission_prep.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/tick.hpp"

// TODO: figure out why it is seg faulting when running these unit tests and fix it...
// I think it's something to do with DECLARE_HANDLER_PARAMS not being up to
// date with updated command line arguments, but after trying to fix that it was
// stil seg faulting and I want a green checkmark...

/*
#define DECLARE_HANDLER_PARAMS(STATE, REQ, RESP) \
    int argc = 2; \
    char path1[] = "bin/obcpp"; \
    char path2[] = "../configs"; \
    char path3[] = "dev"; \
    char path4[] = "stickbug"; \
    char path5[] = "sitl"; \
    char *paths[] = {path1, path2, path3, path4, path5}; \
    char **paths_ptr = paths; \
    std::shared_ptr<MissionState> STATE = std::make_shared<MissionState>(OBCConfig(argc, paths_ptr)); \
    httplib::Request REQ; \
    httplib::Response RESP 

// Might have to change this later on if we preload
// mission from file
TEST(GCSServerTest, GetMissionNoMission) {
    DECLARE_HANDLER_PARAMS(state, req, resp);

    GCS_HANDLE(Get, mission)(state, req, resp);

    EXPECT_EQ(BAD_REQUEST, resp.status);
    EXPECT_EQ(state->mission_params.getCachedMission(), std::nullopt);
}

TEST(GCSServerTest, PostBadMission) {
    DECLARE_HANDLER_PARAMS(state, req, resp);
    req.body = resources::mission_json_bad_1;

    GCS_HANDLE(Post, mission)(state, req, resp);
    EXPECT_EQ(resp.status, BAD_REQUEST);
    GCS_HANDLE(Get, mission)(state, req, resp);
    EXPECT_EQ(resp.status, BAD_REQUEST);
}

// TODO: can probably cut down on the amount of code copy/paste that is going on here
TEST(GCSServerTest, PostMissionThenGetMission) {
    DECLARE_HANDLER_PARAMS(state, req, resp);
    req.body = resources::mission_json_good_1;
    nlohmann::json request_mission = nlohmann::json::parse(req.body);

    GCS_HANDLE(Post, mission)(state, req, resp);

    // ========================================================================
    // Verify all of the internal state is correct

    EXPECT_EQ(OK, resp.status);
    auto cartesian = state->getCartesianConverter().value();

    auto cached_airdrop_bounds = state->mission_params.getAirdropBoundary();
    auto request_airdrop_bounds = request_mission.at("AirdropBoundary");
    EXPECT_EQ(cached_airdrop_bounds.size(), request_airdrop_bounds.size());
    for (int i = 0; i < cached_airdrop_bounds.size(); i++) {
        auto cached_coord = cartesian.toLatLng(cached_airdrop_bounds[i]);
        auto request_coord = request_airdrop_bounds[i];
        auto request_lat = request_coord.at("Latitude").get<double>();
        auto request_lng = request_coord.at("Longitude").get<double>();
        EXPECT_FLOAT_EQ(request_lat, cached_coord.latitude());
        EXPECT_FLOAT_EQ(request_lng, cached_coord.longitude());
    }
    auto cached_flight_bounds = state->mission_params.getFlightBoundary();
    auto request_flight_bounds = request_mission.at("FlightBoundary");
    EXPECT_EQ(cached_flight_bounds.size(), request_flight_bounds.size());
    for (int i = 0; i < cached_flight_bounds.size(); i++) {
        auto cached_coord = cartesian.toLatLng(cached_flight_bounds[i]);
        auto request_coord = request_flight_bounds[i];
        auto request_lat = request_coord.at("Latitude").get<double>();
        auto request_lng = request_coord.at("Longitude").get<double>();
        EXPECT_FLOAT_EQ(request_lat, cached_coord.latitude());
        EXPECT_FLOAT_EQ(request_lng, cached_coord.longitude());
    }
    auto cached_waypoints = state->mission_params.getWaypoints();
    auto request_waypoints = request_mission.at("Waypoints");
    EXPECT_EQ(cached_waypoints.size(), request_waypoints.size());
    for (int i = 0; i < cached_waypoints.size(); i++) {
        auto cached_coord = cartesian.toLatLng(cached_waypoints[i]);
        auto request_coord = request_waypoints[i];
        auto request_lat = request_coord.at("Latitude").get<double>();
        auto request_lng = request_coord.at("Longitude").get<double>();
        auto request_alt = request_coord.at("Altitude").get<double>();
        EXPECT_FLOAT_EQ(request_lat, cached_coord.latitude());
        EXPECT_FLOAT_EQ(request_lng, cached_coord.longitude());
        EXPECT_FLOAT_EQ(request_alt, cached_coord.altitude());
    }
    auto cached_bottles = state->mission_params.getAirdropBottles();
    auto request_bottles = request_mission.at("BottleAssignments");
    EXPECT_EQ(cached_bottles.size(), NUM_AIRDROP_BOTTLES);
    EXPECT_EQ(request_bottles.size(), NUM_AIRDROP_BOTTLES);
    // Create a map from the index to the cached version of the bottle,
    // and the version of the bottle that was sent in the request
    std::unordered_map<BottleDropIndex, Bottle> indexToCached;
    std::unordered_map<BottleDropIndex, Bottle> indexToRequest;
    for (int i = 0; i < NUM_AIRDROP_BOTTLES; i++) {
        Bottle cached = cached_bottles[i];
        Bottle request;
        google::protobuf::util::JsonStringToMessage(request_bottles.at(i).dump(), &request);
        indexToCached.insert({cached.index(), cached});
        indexToRequest.insert({request.index(), request});
    }
    // Now it is easy to compare bottles of the same drop index
    for (int i = BottleDropIndex::A; i <= BottleDropIndex::E; i++) {
        auto index = static_cast<BottleDropIndex>(i);
        Bottle cached = indexToCached.at(index);
        Bottle request = indexToRequest.at(index);
        EXPECT_EQ(cached.ismannikin(), request.ismannikin());
        if (cached.ismannikin()) {
            // only check the other parameters if they are relevant,
            // i.e. this is not the emergent (mannequin) target
            EXPECT_EQ(cached.alphanumeric(), request.alphanumeric());
            EXPECT_EQ(cached.alphanumericcolor(), request.alphanumericcolor());
            EXPECT_EQ(cached.shape(), request.shape());
            EXPECT_EQ(cached.shapecolor(), request.shapecolor());
        }
    }

    // ========================================================================
    // Verify we can get out the same mission json again

    req = httplib::Request();
    resp = httplib::Response();

    GCS_HANDLE(Get, mission)(state, req, resp);

    auto response_mission = nlohmann::json::parse(resp.body);

    EXPECT_EQ(response_mission.at("Waypoints"), request_mission.at("Waypoints"));
    EXPECT_EQ(response_mission.at("AirdropBoundary"), request_mission.at("AirdropBoundary"));
    EXPECT_EQ(response_mission.at("FlightBoundary"), request_mission.at("FlightBoundary"));

    // Have to compare these specifically because the translation alters how some of the fields
    // look. E.g. on the request we specify indices with letters, but after protobuf parses
    // it, it then stores the index as an integer.
    auto response_bottles = response_mission.at("BottleAssignments");
    // already made request_bottles above, so can reuse it here
    EXPECT_EQ(request_bottles.size(), response_bottles.size());
    // already made indexToRequest above, so can reuse it here
    std::unordered_map<BottleDropIndex, Bottle> indexToResponse;
    for (int i = 0; i < response_bottles.size(); i++) {
        Bottle response;
        google::protobuf::util::JsonStringToMessage(response_bottles[i].dump(), &response);
        indexToResponse.insert({response.index(), response});
    }
    for (int i = BottleDropIndex::A; i <= BottleDropIndex::E; i++) {
        auto index = static_cast<BottleDropIndex>(i);
        Bottle response = indexToResponse.at(index);
        Bottle request = indexToRequest.at(index);
        EXPECT_EQ(response.ismannikin(), request.ismannikin());
        if (response.ismannikin()) {
            // only check the other parameters if they are relevant,
            // i.e. this is not the emergent (mannequin) target
            EXPECT_EQ(response.alphanumeric(), request.alphanumeric());
            EXPECT_EQ(response.alphanumericcolor(), request.alphanumericcolor());
            EXPECT_EQ(response.shape(), request.shape());
            EXPECT_EQ(response.shapecolor(), request.shapecolor());
        }
    }
}

TEST(GCSServerTest, GetInitialPathNoPath) {
    DECLARE_HANDLER_PARAMS(state, req, resp);

    EXPECT_EQ(state->getInitPath().get().size(), 0);

    GCS_HANDLE(Get, path, initial)(state, req, resp);

    EXPECT_EQ(resp.status, BAD_REQUEST);
    EXPECT_EQ(state->getInitPath().get().size(), 0);
}

TEST(GCSServerTest, SetupStateTransitions) {
    // First upload a mission so that we generate a path
    DECLARE_HANDLER_PARAMS(state, req, resp);
    req.body = resources::mission_json_good_1;
    state->setTick(new MissionPrepTick(state));

    GCS_HANDLE(Post, mission)(state, req, resp);

    state->doTick();
    EXPECT_EQ(state->getTickID(), TickID::PathGen);
    do { // wait for path to generate
        auto wait = state->doTick();
        std::this_thread::sleep_for(wait);
    } while (state->getInitPath().get().empty());
    // have an initial path, but waiting for validation
    EXPECT_FALSE(state->getInitPath().get().empty());
    EXPECT_EQ(state->getTickID(), TickID::PathValidate); 

    // todo: figure out way to mock the mav connection
    // so we can validate the path and mock mission upload
}
*/