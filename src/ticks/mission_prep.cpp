#include "ticks/mission_prep.hpp"

#include <memory>
#include <string>

#include "utilities/logging.hpp"
#include "core/mission_state.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/ids.hpp"

MissionPrepTick::MissionPrepTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::MissionPrep) {}

std::chrono::milliseconds MissionPrepTick::getWait() const {
    return MISSION_PREP_TICK_WAIT;
}

Tick* MissionPrepTick::tick() {
    if (this->state->mission_params.getCachedMission().has_value()) {
        LOG_F(INFO, "Valid mission configuration detected");

        std::array<Bottle, NUM_AIRDROP_BOTTLES> bottles_to_drop; 

        // Get the airdrop bottles from the mission parameters
        // and copy into an std::array of the same size. Annoying conversion because
        // mission parameters and cv aggregator expect them in different data
        // structures. Could fix by making them both take a vector / both take an
        // array of size NUM_AIRDROP_BOTTLES
        std::copy_n(state->mission_params.getAirdropBottles().begin(),
            NUM_AIRDROP_BOTTLES, bottles_to_drop.begin()); 

        std::string matching_model_dir = this->state->config.cv.matching_model_dir;
        std::string segmentation_model_dir = this->state->config.cv.segmentation_model_dir;
        std::string saliency_model_dir = this->state->config.cv.saliency_model_dir;

        LOG_F(INFO, "Instantiating CV Aggregator with the following models:");
        LOG_F(INFO, "Matching Model: %s", matching_model_dir.c_str());
        LOG_F(INFO, "Segmentation Model: %s", segmentation_model_dir.c_str());
        LOG_F(INFO, "Saliency Model: %s", saliency_model_dir.c_str());

        // Make a CVAggregator instance and set it in the state
        this->state->setCV(std::make_shared<CVAggregator>(Pipeline(
            PipelineParams(bottles_to_drop,
                {}, // TODO: pass in reference images
                matching_model_dir,
                segmentation_model_dir,
                saliency_model_dir))));

        return new PathGenTick(this->state);
    } else {
        return nullptr;
    }
}
