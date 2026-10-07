#ifndef INCLUDE_UTILITIES_DATATYPES_HPP_
#define INCLUDE_UTILITIES_DATATYPES_HPP_

#include <matplot/matplot.h>

#include <cmath>
#include <string>
#include <unordered_set>
#include <vector>

#include "protos/obc.pb.h"
#include "udp_squared/internal/enum.h"
#include "utilities/constants.hpp"
#include "utilities/jsonable.hpp"

struct XYZCoord: jsonable {
    // members left indeterminate; only needed so this can live in a std::array
    XYZCoord() = default;
    XYZCoord(double x, double y, double z) : x(x), y(y), z(z) {}

    /**
     * Checks whether the coordinates of the XYZCoords are identtical
     */
    bool operator==(const XYZCoord &other_point) const;

    /**
     *  Performes vector addition
     *  @see https://mathworld.wolfram.com/VectorAddition.html
     */
    XYZCoord &operator+=(const XYZCoord &other_point) {
        this->x += other_point.x;
        this->y += other_point.y;
        this->z += other_point.z;
        return *this;
    }
    friend XYZCoord operator+(const XYZCoord &lhs, const XYZCoord &rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
    }
    XYZCoord &operator-=(const XYZCoord &other_point) {
        this->x -= other_point.x;
        this->y -= other_point.y;
        this->z -= other_point.z;
        return *this;
    }
    friend XYZCoord operator-(const XYZCoord &lhs, const XYZCoord &rhs) {
        return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
    }

    /**
     * Performs scalar multiplication
     * @see https://mathworld.wolfram.com/ScalarMultiplication.html
     *
     * > the scalar being allowed on the right may be unsafe
     */
    friend XYZCoord operator*(double scalar, const XYZCoord &vector) {
        return {vector.x * scalar, vector.y * scalar, vector.z * scalar};
    }
    friend XYZCoord operator*(const XYZCoord &vector, double scalar) {
        return scalar * vector;
    }

    /**
     * Distance to another XYZCoord
     *
     * @param other point to calculate distance to
     */
    double distanceTo(const XYZCoord &other) const { return (*this - other).norm(); }
    double distanceToSquared(const XYZCoord &other) const {
        return (*this - other).normSquared();
    }

    /**
     * Distance to another XYZCoord in the xy plane
     *
     * Everything pathing measures is flown at a heading, so altitude is not part
     * of the distance it cares about -- a dubins path is a 2D curve.
     *
     * @param other point to calculate distance to
     */
    double distanceTo2D(const XYZCoord &other) const {
        return std::hypot(this->x - other.x, this->y - other.y);
    }

    /**
     * @returns the magnitude of a vector
     * @see https://mathworld.wolfram.com/VectorNorm.html
     */
    double norm() const { return std::sqrt(this->normSquared()); }
    double normSquared() const {
        return this->x * this->x + this->y * this->y + this->z * this->z;
    }

    XYZCoord normalized() const {
        const double magnitude = this->norm();

        if (magnitude == 0) {
            return *this;
        }

        return (1 / magnitude) * (*this);
    }

    nlohmann::json to_json();

    double x;
    double y;
    double z;
};

struct RRTPoint {
    // members left indeterminate; only needed so this can live in a std::array
    RRTPoint() = default;
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

// Because this is a protos class, mildly inconvenient to construct it
// so we have our own "constructor" here
GPSCoord makeGPSCoord(double lat, double lng, double alt);

using Polygon = std::vector<XYZCoord>;
using Polyline = std::vector<XYZCoord>;

using GPSProtoVec = google::protobuf::RepeatedPtrField<GPSCoord>;

std::string AirdropTypesToString(const AirdropType& object);


#endif  // INCLUDE_UTILITIES_DATATYPES_HPP_
