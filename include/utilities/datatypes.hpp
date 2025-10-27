#ifndef INCLUDE_UTILITIES_DATATYPES_HPP_
#define INCLUDE_UTILITIES_DATATYPES_HPP_

#include <matplot/matplot.h>

#include <string>
#include <unordered_set>
#include <vector>

#include "protos/obc.pb.h"
#include "udp_squared/internal/enum.h"
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

std::string AirdropTypesToString(const AirdropType& object);


#endif  // INCLUDE_UTILITIES_DATATYPES_HPP_
