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
            
        // Get the airdrop bottles from the mission parameters
        bottlesToDropV = state->mission_params.getAirdropBottles();

        // Fill Bottles array because Aggregator wants that idrk why
        std::copy_n(bottlesToDropV.begin(), NUMBOTTLES, bottlesToDrop.begin()); 

        // TODO: Add path config to the models
        // matching model can be downloaded from here: https://drive.google.com/drive/folders/1ciDfycNyJiLvRhJhwQZoeKH7vgV6dGHJ?usp=drive_link
        matchingModelPath = "../models/target_siamese_1.pt";
        // segmentation model can be downloaded from here: https://drive.google.com/file/d/1U2EbfJFzcjVnjTuD6ud-bIf8YOiEassf/view?usp=drive_link
        segmentationModelPath = "../models/fcn.pth";

        saliencyModelPath = "../models/torchscript_19.pth";

        // Initalizes pipeline 
        pipeline = std::make_unique<Pipeline>(PipelineParams(bottlesToDrop, referenceImages, matchingModelPath, segmentationModelPath, saliencyModelPath));

        // Make a CVAggregator instance and set it in the state
        this->state->setCV(std::make_shared<CVAggregator>(pipeline));

        LOG_F(INFO, "Valid mission configuration detected");
        return new PathGenTick(this->state);
    } else {
        return nullptr;
    }
}
