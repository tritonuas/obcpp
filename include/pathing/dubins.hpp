#ifndef PATHING_DUBINS_HPP_
#define PATHING_DUBINS_HPP_

#include "Eigen" 

struct DubinsPath {
    DubsinPath(double center_1, double center_2, double straight_dist)
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

/*
 * [TODO] write descritpion
 */
Eigen::Vector2d findOrthogonalVector2D(const Eigen::Vector2d& vector) {
    Eigen::Vector2d orthogonalVector(-vector[1], vector[0]);

    return orthogonalVector;
}


class Dubins {
    /*
        TODO: 
            - implement a package that implements LINALG
            - copy the python code from the other obc repo
    */
public:
    Dubins(double radius, double point_separation) 
        : radius(radius), 
        point_separation(point_separation) 
        {}
    
private:
    double radius;
    double point_seperation;
    
};

#endif // PATHING_DUBINS_HPP_
