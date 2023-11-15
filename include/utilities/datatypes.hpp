#ifndef UTILITIES_DATATYPES_HPP_
#define UTILITIES_DATATYPES_HPP_

#include <vector>
#include <matplot/matplot.h>

struct GPSCoord {
    GPSCoord(double lat, double lon, double alt)
        :lat(lat), lon(lon), alt(alt) {}

    double lat;
    double lon;
    double alt;
};

struct XYZCoord {
    XYZCoord(double x, double y, double z, double psi = 0)
        :x(x), y(y), z(z), psi(psi), color(matplot::color::black) {}

    XYZCoord(double x, double y, double z, double psi, matplot::color color)
        :x(x), y(y), z(z), psi(psi), color(color) {}

    double x;
    double y;
    double z;
    double psi;
    matplot::color color;
};
 
class Polygon : public std::vector<XYZCoord> {
    public:
        Polygon(matplot::color color);

        matplot::color getColor() const;
    private:
        matplot::color color;
};

class Polyline: public std::vector<XYZCoord> {
    public:
        Polyline(matplot::color color);

        matplot::color getColor() const;
    private:
        matplot::color color;
};

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