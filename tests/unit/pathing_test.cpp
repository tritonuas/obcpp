#include <google/protobuf/util/json_util.h>
#include <gtest/gtest.h>

#include <cmath>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <thread>
#include <unordered_map>

#include "core/mission_state.hpp"
#include "network/gcs.hpp"
#include "network/gcs_macros.hpp"
#include "network/gcs_routes.hpp"
#include "pathing/plotting.hpp"
#include "pathing/static.hpp"
#include "resources/json_snippets.hpp"
#include "ticks/mission_prep.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/tick.hpp"
#include "utilities/http.hpp"
#include "utilities/obc_config.hpp"

#include "handler_params.hpp"

// TODO: fails fom the tick ever switching
// copies over code verbatim from the gcs test, and then generates a path
// TEST(StaticPathingTest, RRTTest) {
//     // First upload a mission so that we generate a path
//     DECLARE_HANDLER_PARAMS(state, req, resp);
//     req.body = resources::mission_json_2020;
//     state->setTick(new MissionPrepTick(state));

//     GCS_HANDLE(Post, mission)(state, req, resp);

//     state->doTick();

//     EXPECT_EQ(state->getTickID(), TickID::PathGen);
//     do {  // wait for path to generate
//         auto wait = state->doTick();
//         std::this_thread::sleep_for(wait);
//     } while (state->getInitPath().get().empty());
//     // have an initial path, but waiting for validation
//     EXPECT_FALSE(state->getInitPath().get().empty());
//     EXPECT_EQ(state->getTickID(), TickID::PathValidate);

//     // actually new test
//     // validate the path
//     Environment env(state->mission_params.getFlightBoundary(), {}, {}, {}, {});

//     std::vector<GPSCoord> path = state->getInitPath().get();

//     for (GPSCoord &point : path) {
//         const XYZCoord xyz_point = state->getCartesianConverter().value().toXYZ(point);

//         EXPECT_TRUE(env.isPointInBounds(xyz_point));
//     }

//     // PathingPlot plotter("pathing_output", state->config.getFlightBoundary(),
//     //                     state->config.getAirdropBoundary(), state->config.getWaypoints());

//     // std::vector<XYZCoord> path_coords;

//     // for (auto wpt : path) {
//     //     path_coords.push_back(state->getCartesianConverter().value().toXYZ(wpt));
//     // }

//     // plotter.addFinalPolyline(path_coords);
//     // plotter.output("unit_test_path", PathOutputType::BOTH);
// }

/**
 * Checks final angles in each axis and quadrant (8 checks)
 */
TEST(StaticPathingTest, FinalAngleQuadrant) {
    DECLARE_HANDLER_PARAMS(state, req, resp);

    GPSProtoVec bounds;
    *bounds.Add() = makeGPSCoord(0.0, 0.0, 0.0);
    *bounds.Add() = makeGPSCoord(1.0, 1.0, 0.0);
    CartesianConverter<GPSProtoVec> converter(bounds);
    state->setCartesianConverter(converter);

    struct TestCheck {
        double dx;
        double dy;
        double expected_angle;
    };
    
    std::vector<TestCheck> checks = {
        {1.0, 0.0, 0.0},
        {0.0, 1.0, M_PI / 2.0},
        {-1.0, 0.0, M_PI},
        {0.0, -1.0, -M_PI / 2.0},
        {1.0, 1.0, M_PI / 4.0},
        {-1.0, 1.0, 3.0 * M_PI / 4.0},
        {-1.0, -1.0, -3.0 * M_PI / 4.0},
        {1.0, -1.0, -M_PI / 4.0}
    };

    XYZCoord start_xyz(0.0, 0.0, 0.0);
    GPSCoord start_gps = converter.toLatLng(start_xyz);

    for (const auto& check : checks) {
        XYZCoord end_xyz(check.dx, check.dy, 0.0);
        GPSCoord end_gps = converter.toLatLng(end_xyz);

        MissionPath path(MissionPath::Type::FORWARD, {start_gps, end_gps});

        double calculated_angle = calculateFinalAngle(path, state->getCartesianConverter());
        
        if (check.expected_angle == M_PI && calculated_angle < 0) {
            EXPECT_NEAR(-M_PI, calculated_angle, 0.001);
        } else {
            EXPECT_NEAR(check.expected_angle, calculated_angle, 0.001);
        }
    }
}

/**
 * Verify our implementation of degenrate paths
 */
TEST(StaticPathingTest, FinalAngleLessThanTwoPoints) {
    DECLARE_HANDLER_PARAMS(state, req, resp);

    GPSProtoVec bounds;
    *bounds.Add() = makeGPSCoord(0.0, 0.0, 0.0);
    *bounds.Add() = makeGPSCoord(1.0, 1.0, 0.0);
    CartesianConverter<GPSProtoVec> converter(bounds);
    state->setCartesianConverter(converter);

    XYZCoord start_xyz(0.0, 0.0, 0.0);
    GPSCoord start_gps = converter.toLatLng(start_xyz);

    MissionPath empty_path;
    EXPECT_EQ(0.0, calculateFinalAngle(empty_path, state->getCartesianConverter()));

    MissionPath one_point_path(MissionPath::Type::FORWARD, {start_gps});
    EXPECT_EQ(0.0, calculateFinalAngle(one_point_path, state->getCartesianConverter()));
}


/**
 * Makes sure last two points are taken
 */
TEST(StaticPathingTest, FinalAngleLastTwoPoints) {
    DECLARE_HANDLER_PARAMS(state, req, resp);

    GPSProtoVec bounds;
    *bounds.Add() = makeGPSCoord(0.0, 0.0, 0.0);
    *bounds.Add() = makeGPSCoord(1.0, 1.0, 0.0);
    CartesianConverter<GPSProtoVec> converter(bounds);
    state->setCartesianConverter(converter);

    XYZCoord pt1_xyz(0.0, 0.0, 0.0);
    XYZCoord pt2_xyz(1.0, -1.0, 0.0);
    XYZCoord pt3_xyz(1.0, 1.0, 0.0); 
    
    GPSCoord pt1 = converter.toLatLng(pt1_xyz);
    GPSCoord pt2 = converter.toLatLng(pt2_xyz);
    GPSCoord pt3 = converter.toLatLng(pt3_xyz);

    MissionPath path(MissionPath::Type::FORWARD, {pt1, pt2, pt3});
    
    // final direction is from (1, -1) to (1, 1) -> dx=0, dy=2 -> M_PI / 2.0
    double expected_angle = M_PI / 2.0; 
    EXPECT_NEAR(expected_angle, calculateFinalAngle(path, state->getCartesianConverter()), 0.001);
}

/**
 * Accuracy test on Dubins
 * 
 * @param Separation
 */
TEST(StaticPathingTest, FinalAngleDubinsPath) {
    DECLARE_HANDLER_PARAMS(state, req, resp);

    GPSProtoVec bounds;
    *bounds.Add() = makeGPSCoord(0.0, 0.0, 0.0);
    *bounds.Add() = makeGPSCoord(1.0, 1.0, 0.0);
    CartesianConverter<GPSProtoVec> converter(bounds);
    state->setCartesianConverter(converter);

    const double EXPECTED_END_ANGLE = M_PI / 2.0;

    Dubins dubins(30.0, 30.0);
    RRTPoint start_pt(XYZCoord(0.0, 0.0, 0.0), 0.0);
    RRTPoint end_pt(XYZCoord(300.0, 300.0, 0.0), EXPECTED_END_ANGLE);

    // Dubins Path
    std::vector<XYZCoord> dubins_xyz = dubins.dubinsPath(start_pt, end_pt);
    std::vector<GPSCoord> dubins_gps;
    for (const auto& xyz : dubins_xyz) {
        dubins_gps.push_back(converter.toLatLng(xyz));
    }
    MissionPath path(MissionPath::Type::FORWARD, dubins_gps);

    double calculated_angle = calculateFinalAngle(path, state->getCartesianConverter());

    // Allow some tolerance due to point separation (10 degrees --> 0.174 rad)
    EXPECT_NEAR(EXPECTED_END_ANGLE, calculated_angle, 0.174);
}