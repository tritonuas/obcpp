#ifndef UTILITIES_CARTESIAN_HPP_
#define UTILITIES_CARTESIAN_HPP_
#define _USE_MATH_DEFINES
#include <algorithm>
#include <math.h>

class CartesianConverter {
    /*
        DONE:
        Function that takes GPSCoord and returns XYZCoord
        Function that takes XYZCoord and returns GPSCoord
    */

    private:
        // Keep all coordinates as tuples. 
        // Keep the center point coordinates as a pair so that it may be updated later.
        std::pair<double,double> center;
        double EARTH_RADIUS = 6371008.7714; // Mean radius of the Earth as approximated by the IUGG.

    public:
        /* 
        * Constructor that takes in (center point) in tuple(double,double) format.
        */
        CartesianConverter(std::tuple<double,double> center_point){
            center = std::make_pair(std::get<0>(center_point), std::get<1>(center_point));
        };

        /*
        * tuple(x: double, y: double) -> tuple(lat: double, lng: double)
        * Function that tkes XYZCoord and returns GPSCoord.
        * Will return only positive coordinates, as angles cannot be uniquely converted.
        */
        std::tuple<double,double> xy_to_latlng(double dest_x, double dest_y){
            double x = reverseDistanceBetween(std::make_tuple(center.first, center.second), std::make_tuple(dest_x, center.second));
            double y = reverseDistanceBetween(std::make_tuple(center.first, center.second), std::make_tuple(center.first, dest_y));
            return std::make_tuple(x, y);
        }
        double reverseDistanceBetween(std::tuple<double,double> from, std::tuple<double,double> to){
            return toDegrees(distanceRadians(std::get<0>(from) / EARTH_RADIUS, std::get<1>(from) / EARTH_RADIUS, std::get<0>(to) / EARTH_RADIUS, std::get<1>(to) / EARTH_RADIUS));
        }
        // Radians to Degrees
        double toDegrees(double x){
            return x * (180 / M_PI);
        }

        /*
        * tuple(lat: double, lng: double) -> tuple(x: double, y: double)
        * Function that tkes GPSCoord and returns XYZCoord.
        * Will return only positive XY meters, as angles cannot be uniquely converted.
        */
        std::tuple<double,double> latlng_to_xy(double dest_latitude, double dest_longitude){

            // Compute the distance between the center and destination point's latitudes and longitudes.
            double x = computeDistanceBetween(std::make_tuple(center.first, center.second), std::make_tuple(dest_latitude, center.second));
            double y = computeDistanceBetween(std::make_tuple(center.first, center.second), std::make_tuple(center.first, dest_longitude));

            return std::make_tuple(x, y);
        }

        /* 
        * Calculate final distance between two tuple (latitude, longitude) LatLongs.
        */
        double computeDistanceBetween(std::tuple<double,double> from, std::tuple<double,double> to){
            return computeAngleBetween(from, to) * EARTH_RADIUS;
        }
        /*
        * Calculate radian angle on the unit circle between two tuple (latitude, longitude) LatLongs.
        */
        double computeAngleBetween(std::tuple<double,double> from, std::tuple<double,double> to){
            return distanceRadians(toRadians(std::get<0>(from)), toRadians(std::get<1>(from)), toRadians (std::get<0>(to)), toRadians(std::get<1>(to)));
        }
        /*
        * Calculate radian distance on the unit circle between two (radian, radian) locations on the unit circle.
        */
        double distanceRadians(double lat1, double lng1, double lat2, double lng2) {
            return arcHav(havDistance(lat1, lat2, lng1 - lng2));
        }

        /*
        * Degrees to radians.
        */
        double toRadians(double x){
            return x * (M_PI / 180);
        }
        /*
        * Haversine formula
        * See http://www.movable-type.co.uk/scripts/latlong.html
        */
        double hav(double x) {
            return sin(x * 0.5) * sin(x * 0.5);
        }
        /*
        * Inverse haversine formula
        */
        double arcHav(double x) {
            return 2 * asin(sqrt(x));
        }
        /* 
        * Haversine distance between two points on the unit square
        */
        double havDistance(double lat1, double lat2, double dLng) {
            return hav(lat1 - lat2) + hav(dLng) * cos(lat1) * cos(lat2);
        }

};

#endif // UTILITIES_CARTESIAN_HPP_