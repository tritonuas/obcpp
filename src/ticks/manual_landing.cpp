#include "ticks/manual_landing.hpp"

#include <memory>

#include "cv/mapping.hpp"
#include "ticks/ids.hpp"
#include "utilities/constants.hpp"

namespace fs = std::filesystem;

ManualLandingTick::ManualLandingTick(std::shared_ptr<MissionState> state)
    : Tick(state, TickID::ManualLanding) {}

std::chrono::milliseconds ManualLandingTick::getWait() const { return MANUAL_LANDING_TICK_WAIT; }

Tick* ManualLandingTick::tick() {
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

        Mapping mapper;
        const int chunk_size = 5;
        const int chunk_overlap = 2;
        const int max_dim = 3000;

        std::cout << "First pass stitching..." << std::endl;
        mapper.firstPass(base_dir.string(), output_dir.string(), chunk_size,
        chunk_overlap, scan_mode, max_dim, true);

        std::cout << "Second pass stitching..." << std::endl;
        mapper.secondPass(output_dir.string(), scan_mode, max_dim, true);
        state->setMappingIsDone(true);

        std::cout << "Mapping complete." << std::endl;
    }

    return nullptr;
}
