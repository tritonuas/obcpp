#include "utilities/datatypes.hpp"

#include <cmath>
/*
 *   Empty in-line comments prevent VSCode auto-formatter screrw with my
 *   preffered way of formatting.
 */

bool XYZCoord::operator==(const XYZCoord &other_point) const {
    return this->x == other_point.x     //
           && this->y == other_point.y  //
           && this->z == other_point.z;
}

XYZCoord &XYZCoord::operator+=(const XYZCoord &other_coord) {
    this->x += other_coord.x;
    this->y += other_coord.y;
    this->z += other_coord.z;
    return *this;
}

XYZCoord operator+(const XYZCoord &lhs, const XYZCoord &rhs) {
    XYZCoord result = lhs;
    result += rhs;
    return result;
}

XYZCoord &XYZCoord::operator-=(const XYZCoord &other_coord) {
    this->x -= other_coord.x;
    this->y -= other_coord.y;
    this->z -= other_coord.z;
    return *this;
}

XYZCoord operator-(const XYZCoord &lhs, const XYZCoord &rhs) {
    XYZCoord result = lhs;
    result -= rhs;
    return result;
}

XYZCoord operator*(double scalar, const XYZCoord &vector) {
    XYZCoord result{
        vector.x * scalar, vector.y * scalar,
        vector.z * scalar  //
    };

    return result;
}

XYZCoord operator*(const XYZCoord &vector, double scalar) {
    return scalar * vector;
}

double XYZCoord::norm() const {
    return sqrt(this->x * this->x    //
                + this->y * this->y  //
                + this->z * this->z);
}

XYZCoord XYZCoord::normalized() const {
    double norm = this->norm();

    if (norm == 0) {
        return *this;
    }

    return (1 / this->norm()) * (*this);
}

Polygon::Polygon(matplot::color color) { this->color = color; }

Polyline::Polyline(matplot::color color)
{
    this->color = color;
}

bool Polygon::pointInBounds(XYZCoord point) const
{
    bool is_inside = false;
    // Initialize with the last point
    const XYZCoord *previous_point = &(*this)[this->size() - 1];

    for (const XYZCoord &current_point : *this)
    {
        // divide by zero covered by the first condition
        if ((previous_point->y > point.y) != (current_point.y > point.y) &&
            (point.x < (current_point.x - previous_point->x) //
                    * (point.y - previous_point->y) //
                    / (current_point.y - previous_point->y) //
                    + previous_point->x))
        {
            is_inside = !is_inside;
        }

        previous_point = &current_point;
    }

    return is_inside;
}
Polyline::Polyline(matplot::color color) { this->color = color; }
