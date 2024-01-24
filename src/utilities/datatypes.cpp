#include "utilities/datatypes.hpp"

#include <cmath>

#include "protos/obc.pb.h"

/*
 *   Empty in-line comments prevent VSCode auto-formatter from moving all the
 *   segmented code to inline.
 */

bool XYZCoord::operator==(const XYZCoord &other_point) const {
    return this->x == other_point.x     //
           && this->y == other_point.y  //
           && this->z == other_point.z;
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
    XYZCoord result{
        vector.x * scalar, vector.y * scalar,
        vector.z * scalar  //
    };

    return result;
}

XYZCoord operator*(const XYZCoord &vector, double scalar) { return scalar * vector; }

double XYZCoord::norm() const {
    return sqrt(this->x * this->x    //
                + this->y * this->y  //
                + this->z * this->z);
}

XYZCoord XYZCoord::normalized() const {
    double norm = this->norm();

    if (norm == 0) {
        return *this;
    }

    return (1 / this->norm()) * (*this);
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
    return (this->coord.x == otherPoint.coord.x && this->coord.y == otherPoint.coord.y &&
            this->coord.z == otherPoint.coord.z && this->psi == otherPoint.psi);
}

double RRTPoint::distanceTo(const RRTPoint &otherPoint) const {
    return std::sqrt(std::pow(this->coord.x - otherPoint.coord.x, 2) +
                     std::pow(this->coord.y - otherPoint.coord.y, 2) +
                     std::pow(this->coord.z - otherPoint.coord.z, 2));
}

GPSCoord makeGPSCoord(double lat, double lng, double alt) {
    GPSCoord coord;
    coord.set_latitude(lat);
    coord.set_longitude(lng);
    coord.set_altitude(alt);
    return coord;
}

Polygon::Polygon(matplot::color color) { this->color = color; }

bool Polygon::isPointInBounds(XYZCoord point) const {
    bool is_inside = false;
    // Initialize with the last point
    const XYZCoord *previous_point = &(*this)[this->size() - 1];

    for (const XYZCoord &current_point : *this) {
        // divide by zero covered by the first condition
        if ((previous_point->y > point.y) != (current_point.y > point.y) &&
            (point.x < (current_point.x - previous_point->x)            //
                               * (point.y - previous_point->y)          //
                               / (current_point.y - previous_point->y)  //
                           + previous_point->x)) {
            is_inside = !is_inside;
        }

        previous_point = &current_point;
    }

    return is_inside;
}
Polyline::Polyline(matplot::color color) { this->color = color; }
