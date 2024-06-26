#include "utilities/datatypes.hpp"

#include <cmath>

#include "pathing/cartesian.hpp"
#include "protos/obc.pb.h"

inline bool floatingPointEquals(double x1, double x2) {
    return std::fabs(x1 - x2) < std::numeric_limits<double>::epsilon();
}

bool XYZCoord::operator==(const XYZCoord &other_point) const {
    return floatingPointEquals(this->x, other_point.x) &&
           floatingPointEquals(this->y, other_point.y) &&
           floatingPointEquals(this->z, other_point.z);
}

XYZCoord &XYZCoord::operator+=(const XYZCoord &other_coord) {
    this->x += other_coord.x;
    this->y += other_coord.y;
    this->z += other_coord.z;
    return *this;
}

XYZCoord operator+(const XYZCoord &lhs, const XYZCoord &rhs) {
    XYZCoord result = lhs;
    result += rhs;
    return result;
}

XYZCoord &XYZCoord::operator-=(const XYZCoord &other_coord) {
    this->x -= other_coord.x;
    this->y -= other_coord.y;
    this->z -= other_coord.z;
    return *this;
}

XYZCoord operator-(const XYZCoord &lhs, const XYZCoord &rhs) {
    XYZCoord result = lhs;
    result -= rhs;
    return result;
}

XYZCoord operator*(double scalar, const XYZCoord &vector) {
    return {vector.x * scalar, vector.y * scalar, vector.z * scalar};
}

XYZCoord operator*(const XYZCoord &vector, double scalar) { return scalar * vector; }

double XYZCoord::distanceTo(const XYZCoord &other) const { return (*this - other).norm(); }
double XYZCoord::distanceToSquared(const XYZCoord &other) const {
    return (*this - other).normSquared();
}

double XYZCoord::norm() const { return sqrt(this->normSquared()); }

double XYZCoord::normSquared() const {
    return this->x * this->x + this->y * this->y + this->z * this->z;
}

XYZCoord XYZCoord::normalized() const {
    double norm = this->norm();

    if (norm == 0) {
        return *this;
    }

    return (1 / norm) * (*this);
}

std::size_t PointHashFunction::operator()(const RRTPoint &point) const {
    unsigned int h1 = std::hash<double>{}(point.coord.x);
    unsigned int h2 = std::hash<double>{}(point.coord.y);
    unsigned int h3 = std::hash<double>{}(point.coord.z);

    unsigned int c1 = 0.5 * (h1 + h2) * (h1 + h2 + 1) + h2;
    unsigned int c2 = 0.5 * (c1 + h3) * (c1 + h3 + 1) + h3;

    return c2;
}

RRTPoint::RRTPoint(XYZCoord point, double psi) : coord{point}, psi{psi} {}

bool RRTPoint::operator==(const RRTPoint &otherPoint) const {
    return this->coord == otherPoint.coord && floatingPointEquals(this->psi, otherPoint.psi);
}

double RRTPoint::distanceTo(const RRTPoint &otherPoint) const {
    return this->coord.distanceTo(otherPoint.coord);
}

double RRTPoint::distanceToSquared(const RRTPoint &otherPoint) const {
    return this->coord.distanceToSquared(otherPoint.coord);
}

GPSCoord makeGPSCoord(double lat, double lng, double alt) {
    GPSCoord coord;
    coord.set_latitude(lat);
    coord.set_longitude(lng);
    coord.set_altitude(alt);
    return coord;
}

std::string ODLCShapeToString(const ODLCShape& shape) {
    switch (shape) {
        case ODLCShape::Circle: return "CIRCLE";
        case ODLCShape::Semicircle: return "SEMICIRCLE";
        case ODLCShape::QuarterCircle: return "QUARTERCIRCLE";
        case ODLCShape::Triangle: return "TRIANGLE";
        case ODLCShape::Rectangle: return "RECTANGLE";
        case ODLCShape::Pentagon: return "PENTAGON";
        case ODLCShape::Star: return "STAR";
        case ODLCShape::Cross: return "CROSS";
        default: return "CIRCLE";
    }
}

std::string ODLCColorToString(const ODLCColor& color) {
    switch (color) {
        case ODLCColor::White: return "WHITE";
        case ODLCColor::Black: return "BLACK";
        case ODLCColor::Red: return "RED";
        case ODLCColor::Blue: return "BLUE";
        case ODLCColor::Green: return "GREEN";
        case ODLCColor::Purple: return "PURPLE";
        case ODLCColor::Brown: return "BROWN";
        case ODLCColor::Orange: return "ORANGE";
        // maybe return optional nullopt here instead of defaulting to WHITE
        // in case of an unknown color
        default: return "WHITE";
    }
}
