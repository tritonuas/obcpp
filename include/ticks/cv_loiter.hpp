#ifndef INCLUDE_TICKS_CV_LOITER_HPP_
#define INCLUDE_TICKS_CV_LOITER_HPP_

#include <memory>
#include <chrono>

#include "ticks/tick.hpp"
#include <opencv2/opencv.hpp>
#include <loguru.hpp>

#include "cv/pipeline.hpp"
#include "cv/aggregator.hpp"
#include "protos/obc.pb.h"


/*
 * Stop taking photos, loiter away from the search zone
 * and wait until CV processing is done. 
 * 
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/cvloiter/
 */
class CVLoiterTick : public Tick {
 public:

    enum class Status {
        None = 0,
        Validated = 1,
        Rejected = 2,
    };


    explicit CVLoiterTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    void setStatus(Status status);

    Tick* tick() override;
 private:
    std::deque<ImageData> flightImages;
    Status status;
    //std::unordered_map<BottleDropIndex, std::shared_ptr<DetectedTarget>> bestMatches;
    
};

#endif  // INCLUDE_TICKS_CV_LOITER_HPP_
