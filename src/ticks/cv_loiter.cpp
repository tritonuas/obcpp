#include "ticks/cv_loiter.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"


CVLoiterTick::CVLoiterTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::CVLoiter) {
        // Get the airdrop bottles from the mission parameters
        bottlesToDrop = state.mission_params.getAirdropBottles();
        // Get the images from the camera
        flightImages = this.state.camera.getAllImages();

        // TODO: Add path config to the models
        // matching model can be downloaded from here: https://drive.google.com/drive/folders/1ciDfycNyJiLvRhJhwQZoeKH7vgV6dGHJ?usp=drive_link
        matchingModelPath = "../models/target_siamese_1.pt";
        // segmentation model can be downloaded from here: https://drive.google.com/file/d/1U2EbfJFzcjVnjTuD6ud-bIf8YOiEassf/view?usp=drive_link
        segmentationModelPath = "../models/fcn.pth";

        // Initalizes pipeline 
        pipeline(PipelineParams(bottlesToDrop, referenceImages, matchingModelPath, segmentationModelPath));

        // TODO: Change to reference mission_state 
        aggregator(pipeline);

        for (ImageData imageData : flightImages) {
            // Runs the pipeline on the image data
            state.getCV().aggregator.runPipeline(imageData);
        }

        // Gets the results from the aggregator
        results = aggregator.getResults();
    }

std::chrono::milliseconds CVLoiterTick::getWait() const {
    return CV_LOITER_TICK_WAIT;
}

Tick* CVLoiterTick::tick() {
    //Tick is called if Search Zone coverage path is finished

// TODO: add config option?
// If the config is set to "manual annotation" for CV, then only transition to AirdropApproach once a signal has been received from the GCS.
// If the config is set to "full automatic" for CV, then transition to AirdropApproach

    //Check if all expected targets are found 
    if (false) {
        return new AirdropPrepTick(this->state);
    } 
    // If not all targets are validated invoke Flysearch again to attept to locate the target
    else if (false) {
        return new FlySearchTick(this->state);
    }

    return nullptr;
}
