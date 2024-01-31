#ifndef INCLUDE_UTILITIES_DATATYPES_HPP_
#define INCLUDE_UTILITIES_DATATYPES_HPP_

#include <matplot/matplot.h>
#include <vector>

#include "utilities/constants.hpp"
#include "protos/obc.pb.h"

struct XYZCoord {
    XYZCoord(double x, double y, double z)
        :x(x), y(y), z(z), color(matplot::color::black) {}

    XYZCoord(double x, double y, double z, matplot::color color)
        :x(x), y(y), z(z), color(color) {}

    /**
     * Checks whether the coordinates of the XYZCoords are identtical
     * 
     * DOES NOT CHECK XYZCoord.color
    */
    bool operator== (const XYZCoord &other_point) const;

    /**
     *  Performes vector addition
     *  @see https://mathworld.wolfram.com/VectorAddition.html
    */
    XYZCoord& operator+= (const XYZCoord &other_point);
    friend XYZCoord operator+ (const XYZCoord &lhs, const XYZCoord &rhs);
    XYZCoord& operator-= (const XYZCoord &other_point);
    friend XYZCoord operator- (const XYZCoord &lhs, const XYZCoord &rhs);

    /**
     * Performs scalar multiplication
     * @see https://mathworld.wolfram.com/ScalarMultiplication.html
     * 
     * > the scalar being allowed on the right may be unsaafr
    */
    friend XYZCoord operator* (double scalar, const XYZCoord &vector);
    friend XYZCoord operator* (const XYZCoord &vector, double scalar);

    /**
     * @returns the magnitude of a vector
     * @see https://mathworld.wolfram.com/VectorNorm.html
    */
    double norm() const;

    XYZCoord normalized() const;

    double x;
    double y;
    double z;
    matplot::color color;
};

// Because this is a protos class, mildly inconvenient to construct it
// so we have our own "constructor" here
GPSCoord makeGPSCoord(double lat, double lng, double alt);

using Polygon = std::vector<XYZCoord>;
using Polyline = std::vector<XYZCoord>;

using GPSProtoVec = google::protobuf::RepeatedPtrField<GPSCoord>;

#endif  // INCLUDE_UTILITIES_DATATYPES_HPP_
