#ifndef INCLUDE_TICKS_MISSION_UPLOAD_HPP_
#define INCLUDE_TICKS_MISSION_UPLOAD_HPP_

#include <memory>
#include <chrono>
#include <string>
#include <future>

#include "ticks/tick.hpp"

/*
 * 
 *
 */
class MissionUploadTick: public Tick {
 public:
    explicit MissionUploadTick(std::shared_ptr<MissionState> state);

    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;

 private:
    std::future<bool> mission_uploaded;
};

#endif  // INCLUDE_TICKS_MISSION_UPLOAD_HPP_
