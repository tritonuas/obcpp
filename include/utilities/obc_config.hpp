#ifndef INCLUDE_UTILITIES_OBC_CONFIG_HPP_
#define INCLUDE_UTILITIES_OBC_CONFIG_HPP_

#include <string>
#include "udp_squared/internal/enum.h"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

struct LoggingConfig {
    std::string dir;
};

struct NetworkConfig {
    struct {
        int port;
    } gcs;
    struct {
        std::string connect;
    } mavlink;
};

struct TakeoffConfig {
    float altitude_m;
};

struct CVConfig {
    std::string matching_model_dir;
    std::string segmentation_model_dir;
    std::string saliency_model_dir;
};

struct RRTConfig {
    int iterations_per_waypoint;  // number of iterations run between two waypoints
    double rewire_radius;         // maximum distance from sampled point to optimize during RRT*
    bool optimize;                // run RRT* if true
    POINT_FETCH_METHODS point_fetch_method;
    bool allowed_to_skip_waypoints;  // if true, will skip waypoints if it can not connect after 1
                                     // RRT iteration
};

struct AirdropSearchConfig {
    double coverage_altitude_m;
    bool optimize;  // whether to ignore the config below and run all ways.
    bool vertical;  // if true, will search in vertical lines
    bool one_way;   // if true, path returned will only be in 1 direction
};

struct AirdropApproachConfig {
    drop_mode drop_method;
    std::unordered_set<int> bottle_ids;
    double drop_angle_rad;
    double drop_altitude_m;
    double guided_drop_distance_m;
    double unguided_drop_distance_m;
};

struct PathingConfig {
    RRTConfig rrt;
    AirdropSearchConfig coverage;
    AirdropApproachConfig approach;
};

struct CameraConfig {
    // either "mock" or "lucid"
    std::string type;
    // directory to save images to
    std::string save_dir;
    struct {
        // directory to randomly pick images from
        // for the mock camera
        std::string images_dir;
    } mock;
    // All comments will reference the nodes for the Triton 200s
    // https://support.thinklucid.com/triton-tri200s/
    struct {
        // Image Format Control (https://support.thinklucid.com/triton-tri200s/#2976)
        std::string sensor_shutter_mode;  // Either "Rolling" or "GlobalReset"

        // Acquisition Control (https://support.thinklucid.com/triton-tri200s/#2934)
        bool acquisition_frame_rate_enable;
        int64_t target_brightness;
        std::string exposure_auto;  // either "Continuous" or "Off"
        double exposure_time;  // manual exposure time. only applies when exposure_auto is "Off"
        std::string exposure_auto_algorithm;  // either "Median" or "Mean"
        double exposure_auto_damping;
        double exposure_auto_upper_limit;
        double exposure_auto_lower_limit;

        // Stream settings
        bool stream_auto_negotiate_packet_size;
        bool stream_packet_resend_enable;

        // Device Control (https://support.thinklucid.com/triton-tri200s/#2959)
        std::string device_link_throughput_limit_mode;  // Either "On" or "Off"
        int64_t device_link_throughput_limit;  // for Triton 200S: max 125,000,000 min 31,250,000

        // Analog Control (https://support.thinklucid.com/triton-tri200s/#2953)
        bool gamma_enable;
        double gamma;
        std::string gain_auto;  // either "Continuous" or "Off"
        double gain_auto_upper_limit;
        double gain_auto_lower_limit;
    } lucid;
};

struct OBCConfig {
    LoggingConfig logging;
    NetworkConfig network;
    TakeoffConfig takeoff;
    CVConfig cv;
    PathingConfig pathing;
    CameraConfig camera;

    // Load user specified config json, or make a new one
    OBCConfig(int argc, char* argv[]);
};

#endif  // INCLUDE_UTILITIES_OBC_CONFIG_HPP_
