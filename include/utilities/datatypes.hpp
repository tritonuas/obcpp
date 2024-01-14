#ifndef UTILITIES_DATATYPES_HPP_
#define UTILITIES_DATATYPES_HPP_

#include <vector>
#include <matplot/matplot.h>

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
        :x(x), y(y), z(z), color(matplot::color::black) {}

    XYZCoord(double x, double y, double z, matplot::color color)
        :x(x), y(y), z(z), color(color) {}

    /**
     * Checks whether the coordinates of the XYZCoords are identtical
     * 
     * DOES NOT CHECK XYZCoord.color
    */
    bool operator== (const XYZCoord &other_point) const;

    /**
     *  Performes vector addition
     *  @see https://mathworld.wolfram.com/VectorAddition.html
    */
    XYZCoord& operator+= (const XYZCoord &other_point);
    friend XYZCoord operator+ (const XYZCoord &lhs, const XYZCoord &rhs);
    XYZCoord& operator-= (const XYZCoord &other_point);
    friend XYZCoord operator- (const XYZCoord &lhs, const XYZCoord &rhs);

    /**
     * Performs scalar multiplication
     * @see https://mathworld.wolfram.com/ScalarMultiplication.html
     * 
     * > the scalar being allowed on the right may be unsaafr
    */
    friend XYZCoord operator* (double scalar, const XYZCoord &vector);
    friend XYZCoord operator* (const XYZCoord &vector, double scalar);

    /**
     * @returns the magnitude of a vector
     * @see https://mathworld.wolfram.com/VectorNorm.html
    */
    double norm() const;

    XYZCoord normalized() const;

    double x;
    double y;
    double z;
    matplot::color color;
};
 
class Polygon : public std::vector<XYZCoord> {
    public:
        explicit Polygon(matplot::color color);

        [[nodiscard]] matplot::color getColor() const;

        /**
         * Determines whether a point ia in this polygon via raycasting. Points
         * on the edge are counted as outside the polygon (to be more 
         * conservative)
         * 
         * @param point ==> given point
         * @return      ==> whether or not the point is in this polygon object
         * @see         ==> https://en.wikipedia.org/wiki/Point_in_polygon
        */
        bool pointInBounds(XYZCoord point) const;

        // [TODO] make a method to augment the polygon to get similar polygons
        // [TODO] something that increases cost based on time in the edge
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