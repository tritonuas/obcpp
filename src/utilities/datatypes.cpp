#include "utilities/datatypes.hpp"

#include <cmath>
#include <limits>
#include <string>

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

nlohmann::json XYZCoord::to_json() {
    json xyz_json = {
        {"x", x},
        {"y", y},
        {"z", z}
    };
    return xyz_json;
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


std::string AirdropTypeObjectsToString(const AirdropType& color) {
    switch (color) {
        case AirdropType::Water: return "WATER";
        case AirdropType::Beacon: return "BEACON";
        // maybe return optional nullopt here instead of defaulting to IDFK
        // in case of an unknown object (Not relevant anymore I don't think)
        default: return "IDFK";
    }
}

