#ifndef INCLUDE_UTILITIES_DATATYPES_HPP_
#define INCLUDE_UTILITIES_DATATYPES_HPP_

#include <matplot/matplot.h>

#include <vector>

#include "protos/obc.pb.h"
#include "utilities/constants.hpp"
#include "airdrop/packet.h"

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

struct AirdropApproachConfig {
    ad_mode drop_mode;
    std::unordered_set<int> bottle_ids;
    double drop_altitude;
    double guided_drop_distance;
    double unguided_drop_distance;
};

#endif  // INCLUDE_UTILITIES_DATATYPES_HPP_
