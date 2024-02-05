#include "pathing/cartesian.hpp"

#include <numbers>
#include <vector>
#include <limits>
#include <cmath>
#include <algorithm>

#include "protos/obc.pb.h"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

template<typename GPSCoords>
CartesianConverter<GPSCoords>::CartesianConverter(GPSCoords boundaries) {
    double min_lat = std::numeric_limits<double>::infinity();
    double max_lat = -min_lat;

    double min_lng = min_lat;
    double max_lng = -min_lng;

    for (auto coord : boundaries) {
        min_lat = std::min(min_lat, coord.latitude());
        min_lng = std::min(min_lng , coord.longitude());

        max_lat = std::max(max_lat, coord.latitude());
        max_lng = std::max(max_lng , coord.longitude());
    }

    this->center.set_altitude(0);
    this->center.set_latitude((min_lat + max_lat) / 2.0);
    this->center.set_longitude((min_lng + max_lng) / 2.0);

    this->latlng_0.set_altitude(this->center.altitude());
    this->latlng_0.set_latitude(this->center.latitude() * std::numbers::pi / 180.0);
    this->latlng_0.set_longitude(this->center.longitude() * std::numbers::pi / 180.0);

    // Essentially what is localizing us to this part of the globe
    // "Standard Parallels"
    this->phi_0 = (min_lat - 1) * std::numbers::pi / 180.0;
    this->phi_1 = (max_lat + 1) * std::numbers::pi / 180.0;
}

template <typename GPSCoords>
GPSCoord CartesianConverter<GPSCoords>::toLatLng(XYZCoord coord) const {
    double n = 0.5 * (std::sin(this->phi_0) + std::sin(this->phi_1));
    double c = std::pow(std::cos(this->phi_0), 2.0) + 2 * n * std::sin(this->phi_0);
    double rho0 = EARTH_RADIUS_METERS /
        n * std::sqrt(c - 2 * n * std::sin(this->latlng_0.latitude()));
    double rho = std::sqrt(std::pow(coord.x, 2.0) +
        std::pow(rho0 - coord.y, 2.0)) / EARTH_RADIUS_METERS;
    double theta = std::atan(coord.x / (rho0 - coord.y));

    double lat = std::asin((c - rho * rho * n * n) / 2.0 / n) * 180.0 / std::numbers::pi;
    double lng = (this->latlng_0.longitude() + theta / n) * 180.0 / std::numbers::pi;

    return makeGPSCoord(lat, lng, coord.z);
}

template <typename GPSCoords>
XYZCoord CartesianConverter<GPSCoords>::toXYZ(GPSCoord coord) const {
    double lat = coord.latitude() * std::numbers::pi / 180.0;
    double lng = coord.longitude() * std::numbers::pi / 180.0;

    double n = 1.0 / 2.0 * (std::sin(this->phi_0) + std::sin(this->phi_1));
    double theta = n * (lng - this->latlng_0.longitude());
    double c = std::pow(std::cos(this->phi_0), 2.0) + 2 * n * std::sin(this->phi_0);
    double rho = EARTH_RADIUS_METERS / n * std::sqrt(c - 2.0 * n * std::sin(lat));
    double rho0 = EARTH_RADIUS_METERS /
        n * std::sqrt(c - 2.0 * n * std::sin(this->latlng_0.latitude()));

    double x = rho * std::sin(theta);
    double y = rho0 - rho * std::cos(theta);
    return XYZCoord(x, y, coord.altitude());
}

template <typename GPSCoords>
std::vector<XYZCoord> CartesianConverter<GPSCoords>::toXYZ(GPSCoords coords) const {
    std::vector<XYZCoord> result;
    result.reserve(coords.size());
    for (auto coord : coords) {
        result.push_back(this->toXYZ(coord));
    }
    return result;
}

template <typename GPSCoords>
GPSCoord CartesianConverter<GPSCoords>::getCenter() const {
    return this->center;
}


// Force it to make template instances since this will be compiled separately from
// the rest of the code
template class CartesianConverter<GPSProtoVec>;
template class CartesianConverter<std::vector<GPSCoord>>;
