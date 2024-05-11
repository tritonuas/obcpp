#ifndef INCLUDE_TICKS_IDS_HPP_
#define INCLUDE_TICKS_IDS_HPP_

#include <unordered_map>

enum class TickID {
    MissionPrep,
    PathGen,
    PathValidate,
    MavUpload,
    Takeoff,
    FlyWaypoints,
    FlySearch,
    CVLoiter,
    AirdropPrep,
    AirdropApproach,
    ManualLanding,
    AutoLanding,
    MissionDone,
    ActiveTakeoff
};

#define _SET_TICK_ID_MAPPING(id) \
    case TickID::id: return #id

constexpr const char* TICK_ID_TO_STR(TickID id) {
    switch (id) {
        _SET_TICK_ID_MAPPING(MissionPrep);
        _SET_TICK_ID_MAPPING(PathGen);
        _SET_TICK_ID_MAPPING(PathValidate);
        _SET_TICK_ID_MAPPING(MavUpload);
        _SET_TICK_ID_MAPPING(Takeoff);
        _SET_TICK_ID_MAPPING(FlyWaypoints);
        _SET_TICK_ID_MAPPING(FlySearch);
        _SET_TICK_ID_MAPPING(CVLoiter);
        _SET_TICK_ID_MAPPING(AirdropApproach);
        _SET_TICK_ID_MAPPING(ManualLanding);
        _SET_TICK_ID_MAPPING(AutoLanding);
        _SET_TICK_ID_MAPPING(MissionDone);
        _SET_TICK_ID_MAPPING(ActiveTakeoff);
        default: return "Unknown TickID";
    }
}

#endif  // INCLUDE_TICKS_IDS_HPP_
