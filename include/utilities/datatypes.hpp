#ifndef UTILITIES_DATATYPES_HPP_
#define UTILITIES_DATATYPES_HPP_

#include <vector>

#include "utilities/constants.hpp"

struct GPSCoord {
    GPSCoord(double lat, double lon, double alt)
        :lat(lat), lon(lon), alt(alt) {}

    double lat;
    double lon;
    double alt;
};

struct XYZCoord {
    XYZCoord(double x, double y, double z)
        :x(x), y(y), z(z) {}

    double x;
    double y;
    double z;
};
 
// In the future maybe make these their own classes with methods, etc...
// should be easyish to migrate because the type names will be the same
using Polygon = std::vector<XYZCoord>;
using Polyline = std::vector<XYZCoord>;

// TODO: these will eventually be redefined in a protobuf, so once the generated protobuf code exists we remove these
enum class ODLCShape {
    Circle, 
    Semicircle, 
    QuarterCircle,
    Triangle,
    Rectangle, 
    Pentagon, 
    Star, 
    Cross
};

enum class ODLCColor {
    White,
    Black, 
    Red, 
    Blue, 
    Green, 
    Purple, 
    Brown, 
    Orange
};

struct CompetitionBottle {
    ODLCColor shapeColor;
    ODLCShape shape;
    ODLCColor alphaColor;
    char alphanumeric;
};

typedef std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES> BottleArray;

#endif // UTILITIES_DATATYPES_HPP_