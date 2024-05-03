#ifndef INCLUDE_TICKS_CV_LOITER_HPP_
#define INCLUDE_TICKS_CV_LOITER_HPP_

#include <memory>
#include <chrono>

#include "ticks/tick.hpp"
#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "cv/pipeline.hpp"
#include "cv/aggregator.hpp"

/*
 * Stop taking photos, loiter away from the search zone
 * and wait until CV processing is done. 
 * 
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/cvloiter/
 */
class CVLoiterTick : public Tick {
 public:
    explicit CVLoiterTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
 private:
    std::array<Bottle> bottlesToDrop;
    std::vector<ImageData> flightImages;
    std::vector<ImageData> referenceImages;

    const std::string matchingModelPath;
    const std::string segmentationModelPath;
    
    Pipeline pipeline;
    CVAggregator aggregator;
    std::shared_ptr<CVResults> results;
};

#endif  // INCLUDE_TICKS_CV_LOITER_HPP_
