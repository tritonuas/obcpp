#ifndef INCLUDE_UTILITIES_OBC_CONFIG_HPP_
#define INCLUDE_UTILITIES_OBC_CONFIG_HPP_

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

#include "udp_squared/internal/enum.h"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

#define CONFIG_VARIANT_MAPPING_T(enum_type) \
    const std::initializer_list<std::pair<std::string, enum_type>>

struct LoggingConfig {
    std::string dir;
};

struct NetworkConfig {
    struct {
        int port;
    } gcs;
    struct {
        std::string connect;
        bool log_params;
        float telem_poll_rate;
    } mavlink;
};

struct TakeoffConfig {
    float altitude_m;
    int payload_size;
};

struct CVConfig {
    std::string yolo_model_dir;
    std::string not_stolen_addr;
    uint16_t not_stolen_port;
};

namespace PointFetchMethod {
enum class Enum {
    NONE,    // check RRT against every node (path optimal, but incredibly slow)
    RANDOM,  // check ~k randomly sampled nodes from the tree.
    NEAREST  // check ~$p$ nodes closest to the sampled node (best performance/time ratio from
             // rudimentary testing)
};
CONFIG_VARIANT_MAPPING_T(Enum)
MAPPINGS = {{"none", Enum::NONE}, {"random", Enum::RANDOM}, {"nearest", Enum::NEAREST}};
};  // namespace PointFetchMethod

struct DubinsConfig {
    double turning_radius;
    double point_separation;
};

struct RRTConfig {
    int iterations_per_waypoint;  // number of iterations run between two waypoints
    double rewire_radius;         // maximum distance from sampled point to optimize during RRT*
    bool optimize;                // run RRT* if true
    PointFetchMethod::Enum point_fetch_method;
    bool allowed_to_skip_waypoints;  // if true, will skip waypoints if it can not connect after 1
                                     // RRT iteration
    bool generate_deviations;
};

namespace AirdropCoverageMethod {
enum class Enum { HOVER, FORWARD };
CONFIG_VARIANT_MAPPING_T(Enum) MAPPINGS = {{"hover", Enum::HOVER}, {"forward", Enum::FORWARD}};
};  // namespace AirdropCoverageMethod

struct AirdropCoverageConfig {
    double altitude_m;
    double camera_vision_m;
    AirdropCoverageMethod::Enum method;
    struct {
        std::size_t pictures_per_stop;
        std::time_t hover_time_s;
    } hover;
    struct {
        bool optimize;  // whether to ignore the config below and run all ways.
        bool vertical;  // if true, will search in vertical lines
        bool one_way;   // if true, path returned will only be in 1 direction
    } forward;
};

namespace AirdropDropMethod {
enum class Enum { GUIDED, UNGUIDED };
CONFIG_VARIANT_MAPPING_T(Enum) MAPPINGS = {{"guided", Enum::GUIDED}, {"unguided", Enum::UNGUIDED}};
};  // namespace AirdropDropMethod

struct AirdropApproachConfig {
    AirdropDropMethod::Enum drop_method;
    std::unordered_set<int> airdrop_ids;
    double drop_angle_rad;
    double drop_altitude_m;
    double guided_drop_distance_m;
    double unguided_drop_distance_m;
};

struct PathingConfig {
    DubinsConfig dubins;
    RRTConfig rrt;
    AirdropCoverageConfig coverage;
    AirdropApproachConfig approach;
};

struct CameraConfig {
    // "mock", "picamera", "picamera-1080p", "picamera-4k", "lucid"
    std::string type;
    // directory to save images to
    std::string save_dir;
    // whether or not to save to save_dir
    bool save_images_to_file;
    struct {
        // port to query images from
        // for the mock camera
        int not_stolen_port;
    } mock;
};

struct MavlinkParametersConfig {
    std::unordered_map<std::string, int> param_map;
};

struct OBCConfig {
    LoggingConfig logging;
    NetworkConfig network;
    TakeoffConfig takeoff;
    CVConfig cv;
    PathingConfig pathing;
    CameraConfig camera;
    MavlinkParametersConfig mavlink_parameters;

    // Load user specified config json, or make a new one
    OBCConfig(int argc, char* argv[]);
};

#endif  // INCLUDE_UTILITIES_OBC_CONFIG_HPP_
