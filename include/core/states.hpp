#ifndef INCLUDE_CORE_STATES_HPP_
#define INCLUDE_CORE_STATES_HPP_

#include <array>
#include <optional>
#include <string>
#include <memory>
#include <utility>

#include "camera/interface.hpp"
#include "cv/pipeline.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

/*
    Interface for an arbitrary mission state.
*/
class MissionState {
 public:
    virtual ~MissionState() = default;
    /*
        Function that runs approx 1 time per second, doing the calculations/checks
        needed for the current phase of the mission.

        Returns the new MissionState if a state change needs to occur. If the optional
        type does not have a value, then no state change needs to occur.
    */
    virtual MissionState* tick() = 0;

    /*
        Plain text name of the current state for display purposes
    */
    virtual std::string getName() = 0;
};

/*
    State for when the system has just been turned on and is waiting for
    mission parameters.
*/
class PreparationState : public MissionState {
 public:
    ~PreparationState() override = default;
    MissionState* tick() override;

    std::string getName() override { return "Mission Preparation"; }

 private:
    std::optional<Polygon> flightBoundary;
    std::optional<Polygon> airdropBoundary;
    std::optional<Polyline> waypoints;
    std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES> bottles;
};

/*
    State for when the plane is searching for ground targets. During this time,
    it is actively taking photos and passing them into the CV pipeline.
*/
class SearchState : public MissionState {
 public:
    // Passing in a unique_ptr to a CameraInterface for dependency
    // injection at runtime. This lets us provide any type of camera to
    // be used during the search state (LUCID, GoPro, mock)
    SearchState(std::unique_ptr<CameraInterface> camera,
                std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES> competitionObjectives)
        : camera(std::move(camera)), pipeline(competitionObjectives) {}

    ~SearchState() override = default;
    MissionState* tick() override;

    std::string getName() override { return "Target Search"; }

 private:
    std::unique_ptr<CameraInterface> camera;

    Pipeline pipeline;
};

#endif  // INCLUDE_CORE_STATES_HPP_
