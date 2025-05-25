#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <thread>
#include <unordered_map>

#include "core/mission_state.hpp"
#include "handler_params.hpp"
#include "network/gcs.hpp"
#include "network/gcs_macros.hpp"
#include "network/gcs_routes.hpp"
#include "pathing/plotting.hpp"
#include "pathing/static.hpp"
#include "ticks/airdrop_approach.hpp"
#include "ticks/mav_upload.hpp"
#include "ticks/mission_prep.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/tick.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/http.hpp"

// Tries to drop airdrop thru mavlink with relay 2
int main(int argc, char* argv[]) {
    // DECLARE_HANDLER_PARAMS(state, req, resp);
    // state->setTick(new MissionPrepTick(state));

    if (argc != 2) {
        LOG_F(ERROR, "Expected use: bin/mavsdk [connection]");
        return 1;
    }

    LOG_S(INFO) << "Attempting to connect at " << argv[1];

    MavlinkClient mav(argv[1]);

    std::shared_ptr<MavlinkClient> mav_ptr = std::make_shared<MavlinkClient>(argv[1]);

    std::optional<airdrop_t> next_airdrop_to_drop;
    AirdropIndex next_airdrop = static_cast<AirdropIndex>(2);

    next_airdrop_to_drop = static_cast<airdrop_t>(next_airdrop);

    triggerAirdrop(mav_ptr, next_airdrop_to_drop.value());

    return 0;
}
