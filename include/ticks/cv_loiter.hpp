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
 * loiter away from the search zone and wait until CV processing is done. 
 * If CV Results are validated, proceed to Airdrop Prep.
 * 
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/cvloiter/
 */

class CVLoiterTick : public Tick {
 public:

    /* 
     * Status of the CV processing
     * (Will be) Modified through GCS Manual Control
    */
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
    Status status;
};

#endif  // INCLUDE_TICKS_CV_LOITER_HPP_
