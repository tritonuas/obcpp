#ifndef UTILITIES_CARTESIAN_HPP_
#define UTILITIES_CARTESIAN_HPP_
#define _USE_MATH_DEFINES
#include <algorithm>
#include <math.h>

class CartesianConverter {
    /*
        DONE:
        Function that takes GPSCoord and returns XYZCoord

        TODO:
        Function that takes XYZCoord and returns GPSCoord
        Getter, setter methods 
    */

    private:
        // Keep all coordinates as tuples. 
        // Keep the center point coordinates as a pair so that it may be updated later.
        std::pair<double,double> center;
        double EARTH_RADIUS = 6371008.7714; // Mean radius of the Earth as approximated by the IUGG.

    public:
        // Constructor that takes in (center point) in latitude and longitude format.
        CartesianConverter(std::tuple<double,double> center_point){
            center = std::make_pair(std::get<0>(center_point), std::get<1>(center_point));
        };

        // GPS to XYZ coords format: (lat: double, lng: double) -> tuple(x: double, y: double)
        std::tuple<double,double> latlng_to_xy(double dest_latitude, double dest_longitude){

            // Compute the distance between the center and destination point's latitudes and longitudes.
            double x = computeDistanceBetween(std::make_tuple(center.first, center.second), std::make_tuple(dest_latitude, center.second));
            double y = computeDistanceBetween(std::make_tuple(center.first, center.second), std::make_tuple(center.first, dest_longitude));

            return std::make_tuple(x, y);
        }

        // Compute final distance between two tuple (latitude, longitude) LatLongs
        double computeDistanceBetween(std::tuple<double,double> from, std::tuple<double,double> to){
            return computeAngleBetween(from, to) * EARTH_RADIUS;
        }
        // Compute angle between two tuple (latitude, longitude) LatLongs from radians on the unit circle
        double computeAngleBetween(std::tuple<double,double> from, std::tuple<double,double> to){
            return distanceRadians(toRadians(std::get<0>(from)), toRadians(std::get<1>(from)), toRadians (std::get<0>(to)), toRadians(std::get<1>(to)));
        }
        // Distance between two points on the unit sphere in radians
        double distanceRadians(double lat1, double lng1, double lat2, double lng2) {
            return arcHav(havDistance(lat1, lat2, lng1 - lng2));
        }

        // Degrees to radians
        double toRadians(double x){
            return x * M_PI / 180;
        }
        // Haversine formula
        // http://www.movable-type.co.uk/scripts/latlong.html
        double hav(double x) {
            double sinHalf = sin(x * 0.5);
            return sinHalf * sinHalf;
        }
        // Inverse haversine formula
        double arcHav(double x) {
            return 2 * asin(sqrt(x));
        }
        // Haversine distance between two points on the unit square
        double havDistance(double lat1, double lat2, double dLng) {
            return hav(lat1 - lat2) + hav(dLng) * cos(lat1) * cos(lat2);
        }
};

#endif // UTILITIES_CARTESIAN_HPP_