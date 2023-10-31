#ifndef PATHING_DUBINS_HPP_
#define PATHING_DUBINS_HPP_

#include <math.h>

#include "../utilities/datatypes.hpp"

#include "Eigen" 

struct DubinsPath {
    DubinsPath(double beta_0, double beta_2, double straight_dist)
        :beta_0(beta_0), beta_2(beta_2), straight_dist(straight_dist) {}
    
    double beta_0;
    double beta_2;
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
*/
double distanceBetween(const Eigen::Vector2d& vector1, const Eigen::Vector2d& vector2) {
    return (vector1 - vector2).norm();
}

/*  
*   
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
 
    /**
    *   @param  point --> current position of the plane (vector) with psi in radians
    *   @param  side --> whether the plane is planning to turn left (L) or right (R)
    *   @return center of a turning circle
    */
    Eigen::Vector2d findCenter(const XYZCoord& point, char side) {
        assert(side == 'L' || side == 'R');

        // creates a right angle between the XYZCoord vector towards the center 
        // left is 90 deg CCW, right is 90 deg CW
        double angle = point.psi + (side == 'L' ? M_PI / 2.0 : -M_PI / 2.0);

        // creates the vector offset from the existing position
        return Eigen::Vector2d(point.x + (std::cos(angle) * _radius),
                               point.y + (std::sin(angle) * _radius));
    }

    /**
    *   @param starting_point   ==> position vector of the plane (only psi is used to ascertain direction)
    *   @param beta             ==> no fucking idea (angle)
    *   @param center           ==> center of the circle
    *   @param path_length      ==> the arc-length along the circle 
    *   @returns                ==> point along circle path
    */
    Eigen::Vector2d circle_arc(const XYZCoord& starting_point, double beta, const Eigen::Vector2d& center, double path_length) {
        // forward angle + [(displacement angle to the right) * -1 IF angle is negative ELSE 1]
        double angle = starting_point.psi + ((path_length / _radius) - M_PI / 2) * std::copysign(1.0, beta);
        // unit vector encoding angle information
        Eigen::Vector2d angle_vector(std::cos(angle), std::sin(angle));
        return center + (angle_vector * _radius);
    }   

private:
    double _radius;
    double _point_separation;
};

#endif // PATHING_DUBINS_HPP_
