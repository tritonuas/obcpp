#ifndef INCLUDE_TICKS_FLY_SEARCH_HPP_
#define INCLUDE_TICKS_FLY_SEARCH_HPP_

#include <memory>
#include <chrono>

#include "ticks/tick.hpp"


/*
 * Take photos and run CV pipeline while flying over
 * search region
 * 
 * See https://tritonuas.github.io/wiki/software/obc/tick_architecture/ticks/flysearch/
 */

class FlySearchTick : public Tick {
 public:
    explicit FlySearchTick(std::shared_ptr<MissionState> state);
    std::chrono::milliseconds getWait() const override;

    Tick* tick() override;
};

#endif  // INCLUDE_TICKS_FLY_SEARCH_HPP_
