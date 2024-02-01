#include "utilities/datatypes.hpp"

#include <cmath>

#include "pathing/cartesian.hpp"
#include "protos/obc.pb.h"


inline bool floatingPointEquals(double x1, double x2) {
    return std::fabs(x1 - x2) < std::numeric_limits<double>::epsilon();
}

bool XYZCoord::operator==(const XYZCoord &other_point) const {
    return floatingPointEquals(this->x, other_point.x) \
           && floatingPointEquals(this->y, other_point.y) \
           && floatingPointEquals(this->z, other_point.z);
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

XYZCoord operator*(const XYZCoord &vector, double scalar) {
    return scalar * vector;
}

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

GPSCoord makeGPSCoord(double lat, double lng, double alt) {
    GPSCoord coord;
    coord.set_latitude(lat);
    coord.set_longitude(lng);
    coord.set_altitude(alt);
    return coord;
}
