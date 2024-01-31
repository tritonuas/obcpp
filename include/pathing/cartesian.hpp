#ifndef INCLUDE_PATHING_CARTESIAN_HPP_
#define INCLUDE_PATHING_CARTESIAN_HPP_

#include <vector>
#include <tuple>
// TODO: figure out what include only gives RepeatedPtrField
#include <google/protobuf/util/json_util.h>

#include "protos/obc.pb.h"
#include "utilities/datatypes.hpp"


// GPSCoords should be a container of GPSCoord protobuf structs
template<typename GPSCoords>
class CartesianConverter {
 public:
    explicit CartesianConverter(GPSCoords boundaries);

    GPSCoord toLatLng(XYZCoord coord) const;
    XYZCoord toXYZ(GPSCoord coord) const;
    std::vector<XYZCoord> toXYZ(GPSCoords coords) const;

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

using CartesianConverterProto = CartesianConverter<google::protobuf::RepeatedPtrField<GPSCoord>>;

#endif  // INCLUDE_PATHING_CARTESIAN_HPP_
