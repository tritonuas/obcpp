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
#include "pathing/static.hpp"
#include "resources/json_snippets.hpp"
#include "ticks/mission_prep.hpp"
#include "ticks/mission_upload.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/tick.hpp"
#include "utilities/http.hpp"

#define DECLARE_HANDLER_PARAMS(STATE, REQ, RESP)                            \
    std::shared_ptr<MissionState> STATE = std::make_shared<MissionState>(); \
    httplib::Request REQ;                                                   \
    httplib::Response RESP

// Might have to change this later on if we preload
// mission from file
TEST(StaticPathingTest, TSETSTSET) {
    DECLARE_HANDLER_PARAMS(state, req, resp);

    GCS_HANDLE(Get, mission)(state, req, resp);

    EXPECT_EQ(BAD_REQUEST, resp.status);
    EXPECT_EQ(state->config.getCachedMission(), std::nullopt);
}
