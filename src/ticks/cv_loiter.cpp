#include "ticks/cv_loiter.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"
#include "ticks/airdrop_prep.hpp"
#include "ticks/fly_search.hpp"


CVLoiterTick::CVLoiterTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::CVLoiter) {
        status = CVLoiterTick::Status::None;
        // // Get the images from the camera
        // flightImages = state->getCamera()->getAllImages();

        // for (ImageData imageData : flightImages) {
        //     // Runs the pipeline on the image data
        //     state->getCV()->runPipeline(imageData);
        // }

        // Gets the results from the aggregator

        // for (DetectedTarget detectedTarget : results.ptr->detected_targets) {
        //     if (detectedTarget.likely_bottle == BottleDropIndex::Undefined) {
        //         // Handle Error how
        //         LOG_F(ERROR, "Unknown target type detected");
        //     } else if (bestMatches.count(detectedTarget.likely_bottle) == 0) { // If the bottle is not in the map, add it
        //         bestMatches[detectedTarget.likely_bottle] = std::make_shared<DetectedTarget>(detectedTarget);
        //     } else {
        //         if (detectedTarget.match_distance < bestMatches[detectedTarget.likely_bottle]->match_distance) {
        //             bestMatches[detectedTarget.likely_bottle] = std::make_shared<DetectedTarget>(detectedTarget);
        //         }
        //     }
        // }
    }

std::chrono::milliseconds CVLoiterTick::getWait() const {
    return CV_LOITER_TICK_WAIT;
}

void CVLoiterTick::setStatus(Status status) {
    this->status = status;
}

Tick* CVLoiterTick::tick() {
    // LockPtr<CVResults> results = state->getCV()->getResults();
    //Tick is called if Search Zone coverage path is finished

    //Check if all expected targets are found 
    if (status == Status::Validated) {
        return new AirdropPrepTick(this->state);
    } 

    // If not all targets are validated invoke Flysearch again to attept to locate the target
    else if (status == Status::Rejected) {
        // TODO: Tell Mav to restart Search Mission
        return new FlySearchTick(this->state);
    }

    return nullptr;
}
