#ifndef PATHING_DUBINS_HPP_
#define PATHING_DUBINS_HPP_

#include <math.h>

#include "../utilities/datatypes.hpp"
#include "Eigen" 

struct DubinsPath {
    DubinsPath(double center_1, double center_2, double straight_dist)
        :center_1(center_1), center_2(center_2), straight_dist(straight_dist) {}
    
    double center_1;
    double center_2;
    double straight_dist;
};

struct RRTOption {
    RRTOption(double length, DubinsPath dubins_path, bool has_straight)
        :length(length), dubins_path(dubins_path), has_straight(has_straight) {}

    double length;
    DubinsPath dubins_path;
    bool has_straight;
};

/**
 *  @param vector 2-vector
 *  @return orthogonal 2-vector to @param vector (always a 90 degree rotation counter-clockwise)
 *  @see https://mathworld.wolfram.com/PerpendicularVector.html
 */
Eigen::Vector2d findOrthogonalVector2D(const Eigen::Vector2d& vector) {
    return Eigen::Vector2d(-vector[1], vector[0]);
}

/**
*   @param vector1 2-vector
*   @param vector2 2-vector
*   @return the magnitude of the displacement vector between the two vectors
*   @see 
*/
double distanceBetween(const Eigen::Vector2d& vector1, const Eigen::Vector2d& vector2) {
    return (vector1 - vector2).norm();
}

/*  
*   Chris thoughts --> I'm stupid so I thought about using convex linear combinations (I guess this is stil one), or adding half of the displacement vector between the two points.
*   [TODO] - write desc.
*
*/
Eigen::Vector2d midpoint(const Eigen::Vector2d& vector1, const Eigen::Vector2d& vector2) {
    return (vector1 + vector2) / 2;
}


class Dubins {
    /*
        TODO: 
            - copy the python code from the other obc repo
    */
public:
    Dubins(double radius, double point_separation)
        : _radius(radius), _point_separation(point_separation) {
        assert(radius > 0);
        assert(point_separation > 0);
    }
 
    // [TODO] none of this is correct lol --> from ChatGPT
    Eigen::Vector2d findCenter(const XYZCoord& point, char side) {
        assert(side == 'L' || side == 'R');
        double angle = point.psi + (side == 'L' ? M_PI / 2.0 : -M_PI / 2.0);
        return Eigen::Vector2d(point.x + std::cos(angle) * _radius,
                               point.y + std::sin(angle) * _radius);
    }

    // [TODO] check if works
    Eigen::Vector2d circle_arc(const XYZCoord& starting_point, double beta, const Eigen::Vector2d& center, double path_length) {
        double angle = starting_point.psi + ((path_length / _radius) - M_PI / 2) * std::copysign(1.0, beta);
        Eigen::Vector2d vect(std::cos(angle), std::sin(angle));
        return center + (vect * _radius);
    }   
private:
    double _radius;
    double _point_separation;

};

#endif // PATHING_DUBINS_HPP_
