#include "network/mavlink.hpp"
#include "ticks/airdrop_approach.hpp"
#include "utilities/logging.hpp"

/** 
 * To run, call `bin/drop_it_like_its_hot [CONFIG]`.
 * 
 * DROP IT LIKE ITS HOT is the test for airdrop signal. For the 2025 
 * competition, our airdrop mechanism is a electro-pelectro-permanent
 * magnet that is triggered by the pixhawk. Therefore, we send a signal to the
 * Pixhawk to drop the payload.
 * 
 * This is to test if the signal is sent successfully (we should set up the 
 * apparatus and see if the epm toggles).
 * 
 * For detailed setup, go look at `mavlink_client.cpp`.
*/
int main(int argc, char* argv[]) {
    if (argc != 5) {
        LOG_F(ERROR, "Expected use: bin/drop_it_like_its_hot [config]");
        return 1;
    }

    // connection to mav
    std::shared_ptr<MavlinkClient> mav_ptr(new MavlinkClient(OBCConfig(argc, argv)));

    // dummy info
    std::optional<airdrop_t> next_airdrop_to_drop;
    AirdropType next_airdrop = static_cast<AirdropType>(2);
    next_airdrop_to_drop = static_cast<airdrop_t>(next_airdrop);

    // drop
    triggerAirdrop(mav_ptr, next_airdrop_to_drop.value());

    return 0;
}
