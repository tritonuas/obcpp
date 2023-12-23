#ifndef INCLUDE_UTILITIES_DATATYPES_HPP_
#define INCLUDE_UTILITIES_DATATYPES_HPP_

#include <matplot/matplot.h>
#include <vector>

struct GPSCoord {
    GPSCoord(double lat, double lon, double alt)
        :lat(lat), lon(lon), alt(alt) {}

    double lat;
    double lon;
    double alt;
};

struct XYZCoord {
    XYZCoord(double x, double y, double z)
        :x(x), y(y), z(z), color(matplot::color::black) {}

    XYZCoord(double x, double y, double z, matplot::color color)
        :x(x), y(y), z(z), color(color) {}

    double x;
    double y;
    double z;
    matplot::color color;
};

class Polygon : public std::vector<XYZCoord> {
 public:
    explicit Polygon(matplot::color color);

    [[nodiscard]] matplot::color getColor() const;
 private:
    matplot::color color{};
};

class Polyline: public std::vector<XYZCoord> {
 public:
    explicit Polyline(matplot::color color);

    [[nodiscard]] matplot::color getColor() const;
 private:
    matplot::color color{};
};

// TODO: these will eventually be redefined in a protobuf,
// so once the generated protobuf code exists we remove these
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

#endif  // INCLUDE_UTILITIES_DATATYPES_HPP_
