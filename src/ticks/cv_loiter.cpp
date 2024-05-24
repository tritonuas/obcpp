#include "ticks/cv_loiter.hpp"

#include <memory>

#include "ticks/ids.hpp"
#include "utilities/constants.hpp"


CVLoiterTick::CVLoiterTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::CVLoiter) {
        // Get the images from the camera
        flightImages = state->getCamera()->getAllImages();

        for (ImageData imageData : flightImages) {
            // Runs the pipeline on the image data
            state->getCV()->runPipeline(imageData);
        }

        // Gets the results from the aggregator
        results = state->getCV()->getResults();

        for (DetectedTarget detectedTarget : results->detected_targets) {
                if (detectedTarget.likely_bottle == BottleDropIndex::Undefined) {
                    // Handle Error how
                    LOG_F(ERROR, "Unknown target type detected");
                } else {
                    if (detectedTarget.match_distance > bestMatches[detectedTarget.likely_bottle]->match_distance) {
                        bestMatches[detectedTarget.likely_bottle] = std::make_shared<DetectedTarget>(detectedTarget);
                    }
                }

        //     switch (detectedTarget.likely_bottle) {
        //         case BottleDropIndex::Undefined:
        //             // Handle Error how
        //             break;
        //         case BottleDropIndex::A:
        //             if (detectedTarget.match_distance > bestMatches[BottleDropIndex::A]->match_distance) {
        //                 bestMatches[BottleDropIndex::A] = std::make_shared<DetectedTarget>(detectedTarget);
        //             }
        //             break;
        //         case BottleDropIndex::B:
        //             if (detectedTarget.match_distance > bestMatches[BottleDropIndex::B]->match_distance) {
        //                 bestMatches[BottleDropIndex::B] = std::make_shared<DetectedTarget>(detectedTarget);
        //             }
        //             break;
        //         case BottleDropIndex::C:
        //             if (detectedTarget.match_distance > bestMatches[BottleDropIndex::C]->match_distance) {
        //                 bestMatches[BottleDropIndex::C] = std::make_shared<DetectedTarget>(detectedTarget);
        //             }
        //             break;
        //         case BottleDropIndex::D:
        //             if (detectedTarget.match_distance > bestMatches[BottleDropIndex::D]->match_distance) {
        //                 bestMatches[BottleDropIndex::D] = std::make_shared<DetectedTarget>(detectedTarget);
        //             }
        //             break;
        //         case BottleDropIndex::E:
        //             if (detectedTarget.match_distance > bestMatches[BottleDropIndex::E]->match_distance) {
        //                 bestMatches[BottleDropIndex::E] = std::make_shared<DetectedTarget>(detectedTarget);
        //             }
        //             break;
        //         default:
        //             LOG_F(ERROR, "Unknown target type detected");
        //             break;
        //     }
        }
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
