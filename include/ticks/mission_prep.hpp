#ifndef INCLUDE_TICKS_MISSION_PREP_HPP_
#define INCLUDE_TICKS_MISSION_PREP_HPP_

#include <memory>
#include <chrono>
#include <string>

#include "ticks/tick.hpp"
#include "cv/pipeline.hpp"
#include "cv/aggregator.hpp"

#define NUMBOTTLES 5

/*
 * Checks every second whether or not a valid mission has been uploaded.
 * Transitions to PathGenTick once it has been generated.
 * 
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/missionprep/
 */
class MissionPrepTick : public Tick {
 public:
    explicit MissionPrepTick(std::shared_ptr<MissionState> state);

    std::vector<Bottle> bottlesToDropV;
    std::array<Bottle, NUMBOTTLES> bottlesToDrop;
    std::vector<std::pair<cv::Mat, BottleDropIndex>> referenceImages;

    std::string matchingModelPath;
    std::string segmentationModelPath;
    std::string saliencyModelPath;
    
    std::unique_ptr<Pipeline> pipeline;

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_MISSION_PREP_HPP_
