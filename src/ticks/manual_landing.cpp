#include "ticks/manual_landing.hpp"

#include <memory>

#include "cv/mapping.hpp"
#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

namespace fs = std::filesystem;

ManualLandingTick::ManualLandingTick(std::shared_ptr<MissionState> state, Tick* next_tick)
    :Tick(state, TickID::ManualLanding), next_tick(next_tick) {}

std::chrono::milliseconds ManualLandingTick::getWait() const { return MANUAL_LANDING_TICK_WAIT; }

Tick* ManualLandingTick::tick() {
    if (state->getDroppedAirdrops().size() >= NUM_AIRDROPS) {
        if (state->getMappingIsDone() == false) {
            // Currently does a two pass at the end
            // Probably should update for next year to optimize it.

            cv::Stitcher::Mode scan_mode = cv::Stitcher::SCANS;
            // Direct stitching of all images in the mapping folder

            fs::path base_dir = "../images/mapping";
            fs::path output_dir = base_dir / "output";

            // Make sure the output directory exists (create if it doesn't)
            if (!fs::exists(output_dir)) {
                fs::create_directories(output_dir);
            }

            // TODO: Change this to be a config setting later
            Mapping mapper;
            const int chunk_size = 5;
            const int chunk_overlap = 2;
            const int max_dim = 3000;

            LOG_F(INFO, "First pass stitching...");
            mapper.firstPass(base_dir.string(), output_dir.string(), chunk_size,
            chunk_overlap, scan_mode, max_dim, true);

            LOG_F(INFO, "Second pass stitching...");
            mapper.secondPass(output_dir.string(), scan_mode, max_dim, true);
            state->setMappingIsDone(true);

            LOG_F(INFO, "Mapping complete.");
        }
    } else {
        // TODO: Test this in mock testflight
        if (!state.get()->getMav()->isArmed()) {
            return next_tick;
        }
    }

    return nullptr;
}
