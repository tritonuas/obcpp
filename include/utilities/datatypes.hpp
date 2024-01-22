#ifndef INCLUDE_UTILITIES_DATATYPES_HPP_
#define INCLUDE_UTILITIES_DATATYPES_HPP_

#include <matplot/matplot.h>

#include <vector>

#include "utilities/constants.hpp"
#include "protos/obc.pb.h"

struct XYZCoord {
    XYZCoord(double x, double y, double z) : x(x), y(y), z(z), color(matplot::color::black) {}

    XYZCoord(double x, double y, double z, matplot::color color) : x(x), y(y), z(z), color(color) {}

    /**
     * Checks whether the coordinates of the XYZCoords are identtical
     *
     * DOES NOT CHECK XYZCoord.color
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
     * > the scalar being allowed on the right may be unsaafr
     */
    friend XYZCoord operator*(double scalar, const XYZCoord &vector);
    friend XYZCoord operator*(const XYZCoord &vector, double scalar);

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

struct RRTPoint {
    RRTPoint(XYZCoord point, double psi);
    /*
     *  Equality overload method for RRTPoint
     */
    bool operator==(const RRTPoint &otherPoint) const;

    double distanceTo(const RRTPoint &otherPoint) const;

    XYZCoord point;
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

class Polygon : public std::vector<XYZCoord> {
 public:
    explicit Polygon(matplot::color color);

    [[nodiscard]] matplot::color getColor() const;

    /**
     * Determines whether a point ia in this polygon via raycasting. Points
     * on the edge are counted as outside the polygon (to be more
     * conservative)
     *
     * @param point ==> given point
     * @return      ==> whether or not the point is in this polygon object
     * @see         ==> https://en.wikipedia.org/wiki/Point_in_polygon
     */
    bool pointInBounds(XYZCoord point) const;

    // [TODO] make a method to augment the polygon to get similar polygons
    // [TODO] something that increases cost based on time in the edge
 private:
    matplot::color color{};
};

class Polyline : public std::vector<XYZCoord> {
 public:
    explicit Polyline(matplot::color color);

    [[nodiscard]] matplot::color getColor() const;

 private:
    matplot::color color{};
};

#endif  // INCLUDE_UTILITIES_DATATYPES_HPP_
