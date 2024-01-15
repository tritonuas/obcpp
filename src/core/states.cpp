#include <iostream>
#include <thread>

#include "core/states.hpp"
#include "cv/pipeline.hpp"

#include <opencv2/opencv.hpp>


MissionState* PreparationState::tick() {
    // TODO: logic to check for takeoff signal, and double check to
    // make sure that all the mission parameters are valid

    std::cout << "tick\n";

    return nullptr;
}

MissionState* SearchState::tick() {
    /* During search section of the mission, we should perform
     * the following tasks on every tick (every few seconds):
     *
     * - Capture photo of ground targets
     *      - At time of capture, tag photos with relevant metadata. This might
     *        include GPS position, airspeed, or attitude. We can query
     *        this information from the pixhawk, but it must be done at the same
     *        time the image is captured.
     * - Pass image into CV pipeline
     *       - See cv/pipeline.cpp for more info
     * - Send all cropped targets predicted by saliency to the GCS for manual
     *   human verification by the GCS operator. Also send the outputs of
     *   target matching and localization. The GCS operator will look at which
     *   matches were made by the CV pipeline and verify their correctness.
     *   If we have enough confidence in our pipeline,  we may not need this
     *   stage. However, it is more important to correctly identify a target
     *   than to have a GCS operator spend more time verifying targets.
     *
     * - Dynamically detect and avoid other aircraft. This includes: taking
     *   photos of the surrounding airspace, passing it into a detection model,
     *   and taking evasive manuevers to move out of another aircraft's flight
     *   path.
     *
     * The previously mentioned tasks are to be completed periodically, however
     * we might need some initialization to kick off the static path that will
     * cover the search zone.
     */

    // Just thinking out loud regarding thread saftey here. With the code below,
    // we will be capturing a new image from the camera and running it through
    // the models of the CV pipeline once on every tick (every second or so).
    // In terms of potential data races here are a few I can think of:
    //  - Interfacing with camera hardware across multiple threads
    //  - Reading model weight files across multiple threads
    //  - Querying the pixhawk for telemetry across multiple threads
    //
    // We can add a few mutexes to the SearchState/Pipeline class to ensure that
    // only one thread can access one of these resources at a time. There might
    // be other things to wrap mutexes around that I can't think of or we'll
    // need once the implementations are written.
    //
    // I had another thought where we could push thread saftey to the
    // implementation of some classes. One class I think would benefit is any of
    // the CameraInterface implemenations. I think it would make more sense for
    // the camera class to ensure that two instances of the camera client can't
    // access the same camera hardware across multiple threads. Maybe we could
    // do the same thing for the MavlinkClinet as well.
    //
    // - Anthony
    std::thread spawn([this]() {
        // TODO: Not sure if we should capture a new image on every tick

        // capture an image with tagged metadata
        camera->takePicture();
        ImageData imageData = camera->getLastPicture();

        // run through pipeline
        PipelineResults results = pipeline.run(imageData);

        // TODO: send to GCS for verification
    });

    // TODO: Need a way to cleanup any running threads when the state changes
    // (maybe on a timeout after searching for too long). This shouldn't go in
    // tick but maybe each state could have a cleanup function that gets called
    // on state change.

    return nullptr;
}
