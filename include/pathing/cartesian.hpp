#ifndef INCLUDE_PATHING_CARTESIAN_HPP_
#define INCLUDE_PATHING_CARTESIAN_HPP_

#include <vector>
#include <tuple>

#include "protos/obc.pb.h"
#include "utilities/datatypes.hpp"

class CartesianConverter {
 public:
    explicit CartesianConverter(google::protobuf::RepeatedPtrField<GPSCoord> boundaries);

    GPSCoord toLatLng(XYZCoord coord) const;
    XYZCoord toXYZ(GPSCoord coord) const;
    std::vector<XYZCoord> toXYZ(google::protobuf::RepeatedPtrField<GPSCoord> coords) const;

    GPSCoord getCenter() const;

 private:

    // Geometric center of the boundaries passed into the constructor,
    GPSCoord center;
    // center but multiplied by pi/180, because this is commonly used in
    // the conversion
    GPSCoord latlng_0;

    // Used in the math, based on the area of the globe that we are
    // localized within
    double phi_0;
    double phi_1;
};

#endif  // INCLUDE_PATHING_CARTESIAN_HPP_
