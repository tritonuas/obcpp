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
