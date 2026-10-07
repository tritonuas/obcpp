#include "core/mission_parameters.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "pathing/cartesian.hpp"
#include "protos/obc.pb.h"
#include "utilities/datatypes.hpp"

namespace {

// somewhere over Maryland, a hundredth of a degree is a bit under a kilometer
const double CENTER_LAT = 38.31;
const double CENTER_LNG = -76.55;

void addCoord(GPSProtoVec* coords, double lat, double lng) {
    GPSCoord* coord = coords->Add();
    coord->set_latitude(lat);
    coord->set_longitude(lng);
    coord->set_altitude(0);
}

// a square centered on the field, reaching out half_width degrees each way
void addSquare(GPSProtoVec* coords, double half_width) {
    addCoord(coords, CENTER_LAT - half_width, CENTER_LNG - half_width);
    addCoord(coords, CENTER_LAT - half_width, CENTER_LNG + half_width);
    addCoord(coords, CENTER_LAT + half_width, CENTER_LNG + half_width);
    addCoord(coords, CENTER_LAT + half_width, CENTER_LNG - half_width);
}

// a mission that is flyable as it stands, for a test to then break one part of
Mission validMission() {
    Mission mission;
    addSquare(mission.mutable_flightboundary(), 0.01);
    addSquare(mission.mutable_airdropboundary(), 0.005);
    addCoord(mission.mutable_waypoints(), CENTER_LAT, CENTER_LNG);
    addCoord(mission.mutable_waypoints(), CENTER_LAT + 0.008, CENTER_LNG - 0.008);
    return mission;
}

std::optional<std::string> upload(const Mission& mission) {
    MissionParameters params;
    return params.setMission(mission, CartesianConverter<GPSProtoVec>(mission.flightboundary()));
}

}  // namespace

/*
 *  MissionParameters::setMission -- a mission that stays inside its own flight
 *  boundary is taken as it is
 */
TEST(MissionParametersTest, MissionInsideTheFlightBoundaryIsAccepted) {
    EXPECT_FALSE(upload(validMission()).has_value());
}

/*
 *  MissionParameters::setMission -- the plane is never asked to fly to a waypoint
 *  it is not allowed to fly to
 */
TEST(MissionParametersTest, WaypointOutsideTheFlightBoundaryIsRejected) {
    Mission mission = validMission();
    addCoord(mission.mutable_waypoints(), CENTER_LAT + 0.02, CENTER_LNG);

    const std::optional<std::string> err = upload(mission);

    ASSERT_TRUE(err.has_value());
    EXPECT_NE(err->find("Waypoint 3"), std::string::npos) << err.value();
    EXPECT_NE(err->find("flight boundary"), std::string::npos) << err.value();
}

/*
 *  MissionParameters::setMission -- an airdrop zone reaching outside of the flight
 *  boundary would send the plane out of it
 */
TEST(MissionParametersTest, AirdropBoundaryOutsideTheFlightBoundaryIsRejected) {
    // bigger than the flight boundary it is supposed to sit inside of
    Mission swallowing = validMission();
    swallowing.clear_airdropboundary();
    addSquare(swallowing.mutable_airdropboundary(), 0.02);

    const std::optional<std::string> swallowing_err = upload(swallowing);
    ASSERT_TRUE(swallowing_err.has_value());
    EXPECT_NE(swallowing_err->find("Airdrop boundary"), std::string::npos)
        << swallowing_err.value();

    // hanging off of one side of it
    Mission overlapping = validMission();
    overlapping.clear_airdropboundary();
    addCoord(overlapping.mutable_airdropboundary(), CENTER_LAT - 0.005, CENTER_LNG);
    addCoord(overlapping.mutable_airdropboundary(), CENTER_LAT - 0.005, CENTER_LNG + 0.03);
    addCoord(overlapping.mutable_airdropboundary(), CENTER_LAT + 0.005, CENTER_LNG + 0.03);
    addCoord(overlapping.mutable_airdropboundary(), CENTER_LAT + 0.005, CENTER_LNG);

    EXPECT_TRUE(upload(overlapping).has_value());
}

/*
 *  MissionParameters::setMission -- every way the mission is wrong is reported at
 *  once, so the operator does not have to fix them one upload at a time
 */
TEST(MissionParametersTest, EveryProblemIsReportedTogether) {
    Mission mission = validMission();
    addCoord(mission.mutable_waypoints(), CENTER_LAT + 0.02, CENTER_LNG);
    mission.clear_airdropboundary();
    addSquare(mission.mutable_airdropboundary(), 0.02);

    const std::optional<std::string> err = upload(mission);

    ASSERT_TRUE(err.has_value());
    EXPECT_NE(err->find("Waypoint"), std::string::npos) << err.value();
    EXPECT_NE(err->find("Airdrop boundary"), std::string::npos) << err.value();
}
