#include <google/protobuf/util/json_util.h>
#include <gtest/gtest.h>

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

#define DECLARE_HANDLER_PARAMS(STATE, REQ, RESP)                            \
    int argc = 2; \
    char path1[] = "bin/obcpp"; \
    char path2[] = "../../../configs/dev-config.json"; \
    char *paths[] = {path1, path2}; \
    char **paths_ptr = paths; \
    std::shared_ptr<MissionState> STATE = std::make_shared<MissionState>(OBCConfig(argc, paths_ptr)); \
    httplib::Request REQ;                                                   \
    httplib::Response RESP

// copies over code verbatim from the gcs test, and then generates a path
TEST(StaticPathingTest, RRTTest) {
    // First upload a mission so that we generate a path
    DECLARE_HANDLER_PARAMS(state, req, resp);
    req.body = resources::mission_json_2020;
    state->setTick(new MissionPrepTick(state));

    GCS_HANDLE(Post, mission)(state, req, resp);

    state->doTick();
    EXPECT_EQ(state->getTickID(), TickID::PathGen);
    do {  // wait for path to generate
        auto wait = state->doTick();
        std::this_thread::sleep_for(wait);
    } while (state->getInitPath().empty());
    // have an initial path, but waiting for validation
    EXPECT_FALSE(state->getInitPath().empty());
    EXPECT_EQ(state->getTickID(), TickID::PathValidate);

    // actually new test
    // validate the path
    Environment env(state->mission_params.getFlightBoundary(), {}, {}, {});

    std::vector<GPSCoord> path = state->getInitPath();

    for (GPSCoord &point : path) {
        const XYZCoord xyz_point = state->getCartesianConverter().value().toXYZ(point);

        EXPECT_TRUE(env.isPointInBounds(xyz_point));
    }

    // PathingPlot plotter("pathing_output", state->config.getFlightBoundary(),
    //                     state->config.getAirdropBoundary(), state->config.getWaypoints());

    // std::vector<XYZCoord> path_coords;

    // for (auto wpt : path) {
    //     path_coords.push_back(state->getCartesianConverter().value().toXYZ(wpt));
    // }

    // plotter.addFinalPolyline(path_coords);
    // plotter.output("unit_test_path", PathOutputType::BOTH);
}
