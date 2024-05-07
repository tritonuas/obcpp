#ifndef INCLUDE_UTILITIES_DATATYPES_HPP_
#define INCLUDE_UTILITIES_DATATYPES_HPP_

#include <matplot/matplot.h>

#include <vector>

#include "protos/obc.pb.h"
#include "utilities/constants.hpp"

struct XYZCoord {
    XYZCoord(double x, double y, double z) : x(x), y(y), z(z) {}

    /**
     * Checks whether the coordinates of the XYZCoords are identtical
     */
    bool operator==(const XYZCoord &other_point) const;

    /**
     *  Performes vector addition
     *  @see https://mathworld.wolfram.com/VectorAddition.html
     */
    XYZCoord &operator+=(const XYZCoord &other_point);
    friend XYZCoord operator+(const XYZCoord &lhs, const XYZCoord &rhs);
    XYZCoord &operator-=(const XYZCoord &other_point);
    friend XYZCoord operator-(const XYZCoord &lhs, const XYZCoord &rhs);

    /**
     * Performs scalar multiplication
     * @see https://mathworld.wolfram.com/ScalarMultiplication.html
     *
     * > the scalar being allowed on the right may be unsafe
     */
    friend XYZCoord operator*(double scalar, const XYZCoord &vector);
    friend XYZCoord operator*(const XYZCoord &vector, double scalar);

    /**
     * Distance to another XYZCoord
     *
     * @param other point to calculate distance to
     */
    double distanceTo(const XYZCoord &other) const;
    double distanceToSquared(const XYZCoord &other) const;

    /**
     * @returns the magnitude of a vector
     * @see https://mathworld.wolfram.com/VectorNorm.html
     */
    double norm() const;
    double normSquared() const;

    XYZCoord normalized() const;

    double x;
    double y;
    double z;
};

struct RRTPoint {
    RRTPoint(XYZCoord point, double psi);
    /*
     *  Equality overload method for RRTPoint
     */
    bool operator==(const RRTPoint &otherPoint) const;

    double distanceTo(const RRTPoint &otherPoint) const;
    double distanceToSquared(const RRTPoint &otherPoint) const;

    XYZCoord coord;
    double psi;
};

// Hash functions for the tree's member variables
class PointHashFunction {
 public:
    /*
     *  Hashes RRTPoint using the Cantor Pairing Function.
     *  Used to add elements to unordered_map nodeMap in RRTTree.
     */
    std::size_t operator()(const RRTPoint &point) const;
};

// Because this is a protos class, mildly inconvenient to construct it
// so we have our own "constructor" here
GPSCoord makeGPSCoord(double lat, double lng, double alt);

using Polygon = std::vector<XYZCoord>;
using Polyline = std::vector<XYZCoord>;

using GPSProtoVec = google::protobuf::RepeatedPtrField<GPSCoord>;

enum POINT_FETCH_METHODS {
    NONE,    // check RRT against every node (path optimal, but incredibly slow)
    RANDOM,  // check ~k randomly sampled nodes from the tree.
    NEAREST  // check ~$p$ nodes closest to the sampled node (best performance/time ratio from
             // rudimentary testing)
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
    bool optimize;  // whether to ignore the config below and run all ways.
    bool vertical;  // if true, will search in vertical lines
    bool one_way;   // if true, path returned will only be in 1 direction
};

struct CameraConfig {
    // either "mock" or "lucid"
    std::string type;
    struct {
        // directory to randomly pick images from 
        // for the mock camera
        std::string images_dir;
    } mock;
    // All comments will reference the nodes for the Triton 200s
    // https://support.thinklucid.com/triton-tri200s/
    struct {
        // Image Format Control (https://support.thinklucid.com/triton-tri200s/#2976)
        std::string sensor_shutter_mode; // Either "Rolling" or "GlobalReset"

        // Acquisition Control (https://support.thinklucid.com/triton-tri200s/#2934)
        bool acquisition_frame_rate_enable;
        int64_t target_brightness;
        std::string exposure_auto; // either "Continuous" or "Off"
        double exposure_time; // manual exposure time. only applies when exposure_auto is "Off"
        std::string exposure_auto_algorithm; // either "Median" or "Mean"
        double exposure_auto_damping;
        double exposure_auto_upper_limit;
        double exposure_auto_lower_limit;

        // Stream settings
        bool stream_auto_negotiate_packet_size;
        bool stream_packet_resend_enable;

        // Device Control (https://support.thinklucid.com/triton-tri200s/#2959)
        std::string device_link_throughput_limit_mode; // Either "On" or "Off"
        int64_t device_link_throughput_limit; // for Triton 200S: max 125,000,000 min 31,250,000

        // Analog Control (https://support.thinklucid.com/triton-tri200s/#2953)
        bool gamma_enable;
        double gamma;
        std::string gain_auto; // either "Continuous" or "Off"
        double gain_auto_upper_limit;
        double gain_auto_lower_limit;
    } lucid;
};

#endif  // INCLUDE_UTILITIES_DATATYPES_HPP_
