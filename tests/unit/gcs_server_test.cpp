#include <gtest/gtest.h>
#include <google/protobuf/util/json_util.h>

#include <memory>
#include <fstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "network/gcs.hpp"
#include "network/gcs_routes.hpp"
#include "core/mission_state.hpp"
#include "utilities/macros.hpp"
#include "resources/json_snippets.hpp"

#define DECLARE_HANDLER_PARAMS \
    std::shared_ptr<MissionState> state = std::make_shared<MissionState>(); \
    httplib::Request req; \
    httplib::Response resp

#define RUN_HANDLER(...) \
    GCS_HANDLE(__VA_ARGS__)(state, req, resp)

// Might have to change this later on if we preload
// mission from file
TEST(GCSServerTest, GetMissionNoMission) {
    DECLARE_HANDLER_PARAMS;

    RUN_HANDLER(Get, mission);

    EXPECT_EQ(BAD_REQUEST, resp.status);
    EXPECT_EQ(state->config.getCachedMission(), std::nullopt);
}

TEST(GCSServerTest, PostMissionThenGetMission) {
    DECLARE_HANDLER_PARAMS;
    req.body = resources::mission_json_good_1;
    nlohmann::json request_mission = nlohmann::json::parse(req.body);

    RUN_HANDLER(Post, mission);

    // ========================================================================
    // Verify all of the internal state is correct
    EXPECT_EQ(OK, resp.status);
    auto cartesian = state->getCartesianConverter().value();

    auto cached_airdrop_bounds = state->config.getAirdropBoundary();
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
    auto cached_flight_bounds = state->config.getFlightBoundary();
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
    auto cached_waypoints = state->config.getWaypoints();
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
    auto cached_bottles = state->config.getAirdropBottles();
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

    RUN_HANDLER(Get, mission);

    auto response_mission = nlohmann::json::parse(resp.body);

    EXPECT_EQ(response_mission.at("Waypoints"), request_waypoints.at("Waypoints"));
    EXPECT_EQ(response_mission.at("AirdropBoundary"), request_waypoints.at("AirdropBoundary"));
    EXPECT_EQ(response_mission.at("FlightBoundary"), request_waypoints.at("FlightBoundary"));

    // Have to compare these specifically because the translation alters how some of the fields
    // look. E.g. on the request we specify indices with letters, but after protobuf parses
    // it, it then stores the index as an integer.
    auto request_bottles = request_mission.at("BottleAssignments");
    auto response_bottles = response_mission.at("BottleAssignments");
    EXPECT_EQ(request_bottles.size(), response_bottles.size());
    std::unordered_map<BottleDropIndex, Bottle> indexToResponse;
    // already made indexToRequest above, so can reuse
    for (int i = 0; i < response_bottles.size(); i++) {
        
    }
}