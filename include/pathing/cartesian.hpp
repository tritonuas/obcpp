#ifndef UTILITIES_CARTESIAN_HPP_
#define UTILITIES_CARTESIAN_HPP_
#define _USE_MATH_DEFINES
#include <map>
#include <string>
#include <iostream>
#include <stdexcept>

class CartesianConverter {
    /*
        TODO:
        Function that takes GPSCoord and returns XYZCoord
        Function that takes XYZCoord and returns GPSCoord 
    */

    private:
        std::map<std::string,float> center;

    public:
        // Constructor that takes in (center point) in latitude and longitude format.
        CartesianConverter(std::map<std::string,float> center_point){
            if((center_point.find("latitude") == center_point.end()) || (center_point.find("longitude") == center_point.end())){
                throw std::invalid_argument("Center point must have the \"latitude\" and \"longitude\" keys.");
            }

            center = center_point;
        };

};

#endif // UTILITIES_CARTESIAN_HPP_