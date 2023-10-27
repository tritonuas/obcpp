#ifndef UTILITIES_DATATYPES_HPP_
#define UTILITIES_DATATYPES_HPP_

#include <vector>

struct GPSCoord {
    GPSCoord(double lat, double lon, double alt)
        :lat(lat), lon(lon), alt(alt) {}

    double lat;
    double lon;
    double alt;
};

struct XYZCoord {
    XYZCoord(double x, double y, double z, double psi = 0)
        :x(x), y(y), z(z), psi(psi) {}

    double x;
    double y;
    double z;
    double psi;
};
 
// In the future maybe make these their own classes with methods, etc...
// should be easyish to migrate because the type names will be the same
typedef std::vector<XYZCoord> Polygon;
typedef std::vector<XYZCoord> Polyline;

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

#endif // UTILITIES_DATATYPES_HPP_