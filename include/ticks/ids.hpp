#ifndef INCLUDE_TICKS_IDS_HPP_
#define INCLUDE_TICKS_IDS_HPP_

#include <unordered_map>

enum class TickID {
    MissionPrep,
    PathGen,
    PathValidate,
    MissionUpload,
    MissionStart,
    EvasionState
};

#define _SET_TICK_ID_MAPPING(id) \
    case TickID::id: return #id

constexpr const char* TICK_ID_TO_STR(TickID id) {
    switch (id) {
        _SET_TICK_ID_MAPPING(MissionPrep);
        _SET_TICK_ID_MAPPING(PathGen);
        _SET_TICK_ID_MAPPING(PathValidate);
        _SET_TICK_ID_MAPPING(MissionUpload);
        _SET_TICK_ID_MAPPING(MissionStart);
        _SET_TICK_ID_MAPPING(EvasionState);
        default: return "Unknown TickID";
    }
}

#endif  // INCLUDE_TICKS_IDS_HPP_
