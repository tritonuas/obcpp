#ifndef UTILITIES_DATATYPES_HPP_
#define UTILITIES_DATATYPES_HPP_

#include <vector>
#include <matplot/matplot.h>

#include "utilities/constants.hpp"

#include <Eigen>

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

#endif // UTILITIES_DATATYPES_HPP_