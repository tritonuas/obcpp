#ifndef INCLUDE_TICKS_IDS_HPP_
#define INCLUDE_TICKS_IDS_HPP_

#include <unordered_map>

enum class TickID {
    MissionPrep,
    PathGen,
    PathValidate,
    MissionUpload,
    Takeoff,
    FlyWaypoints,
    FlySearch,
    CVLoiter,
    AirdropApproach,
    ManualLanding,
    AutoLanding,
    MissionDone
};

#define _SET_TICK_ID_MAPPING(id) \
    case TickID::id: return #id

constexpr const char* TICK_ID_TO_STR(TickID id) {
    switch (id) {
        _SET_TICK_ID_MAPPING(MissionPrep);
        _SET_TICK_ID_MAPPING(PathGen);
        _SET_TICK_ID_MAPPING(PathValidate);
        _SET_TICK_ID_MAPPING(MissionUpload);
        _SET_TICK_ID_MAPPING(Takeoff);
        _SET_TICK_ID_MAPPING(FlyWaypoints);
        _SET_TICK_ID_MAPPING(FlySearch);
        _SET_TICK_ID_MAPPING(CVLoiter);
        _SET_TICK_ID_MAPPING(AirdropApproach);
        _SET_TICK_ID_MAPPING(ManualLanding);
        _SET_TICK_ID_MAPPING(AutoLanding);
        _SET_TICK_ID_MAPPING(MissionDone);
        default: return "Unknown TickID";
    }
}

#endif  // INCLUDE_TICKS_IDS_HPP_
