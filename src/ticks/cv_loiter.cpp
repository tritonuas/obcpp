#include "ticks/cv_loiter.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "cv/pipeline.hpp"
#include "cv/aggregator.hpp"

CVLoiterTick::CVLoiterTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::CVLoiter) {
        // Get the airdrop bottles from the mission parameters
        std::array<Bottle> bottlesToDrop = state.mission_params.getAirdropBottles();
        // Get the images from the camera
        std::vector<ImageData> flightImages = this.state.camera.getAllImages();

        // TODO: Reference images doesn't exist atm
        std::vector<ImageData> referenceImages;

        // TODO: Add path config to the models
        // matching model can be downloaded from here: https://drive.google.com/drive/folders/1ciDfycNyJiLvRhJhwQZoeKH7vgV6dGHJ?usp=drive_link
        const std::string matchingModelPath = "../models/target_siamese_1.pt";
        // segmentation model can be downloaded from here: https://drive.google.com/file/d/1U2EbfJFzcjVnjTuD6ud-bIf8YOiEassf/view?usp=drive_link
        const std::string segmentationModelPath = "../models/fcn.pth";

        // Initalizes pipeline 
        Pipeline pipeline(PipelineParams(bottlesToDrop, referenceImages, matchingModelPath, segmentationModelPath));

        // Creates a CV aggregator instance
        CVAggregator aggregator(pipeline);

        for (ImageData imageData : flightImages) {
            // Runs the pipeline on the image data
            aggregator.runPipeline(imageData);

    }

std::chrono::milliseconds CVLoiterTick::getWait() const {
    return CV_LOITER_TICK_WAIT;
}

Tick* CVLoiterTick::tick() {
    //Tick is called if Search Zone coverage path is finished


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
