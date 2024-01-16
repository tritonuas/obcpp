#ifndef INCLUDE_UTILITIES_DATATYPES_HPP_
#define INCLUDE_UTILITIES_DATATYPES_HPP_

#include <matplot/matplot.h>
#include <vector>

#include "utilities/constants.hpp"

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

#endif  // INCLUDE_UTILITIES_DATATYPES_HPP_
