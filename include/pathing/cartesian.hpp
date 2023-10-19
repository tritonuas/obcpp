#ifndef UTILITIES_CARTESIAN_HPP_
#define UTILITIES_CARTESIAN_HPP_
#define _USE_MATH_DEFINES
#include <algorithm>
#include <math.h>
#include <string>
#include <tuple>

/*
* Defines a (lat,lng,alt) or (x,y,z) point on the coordinate plane.
*/
struct point {
    double lat_x;
    double lng_y;
    double alt_z;
};

/*
* Converter for between (lat,lng,alt) and (x,y,z).
*/
class CartesianConverter {
    private:
        point start;
        double EARTH_RADIUS = 6371008.7714; // Mean radius of the Earth as approximated by the IUGG.

    public:
        /* 
        * Constructor that takes in a starting (lat,long,alt) or (x,y,z) point.
        */
        CartesianConverter(struct point *pt){
            start = *pt;
        };

        /*
        * point(x,y,z) -> point(lat,long,alt)
        * Function that tkes XYZCoord and returns GPSCoord.
        * Will return only positive coordinates after distance calculations as angles cannot be uniquely converted.
        * Does not calculate altitude change conversion; returned point will retain starting point altitude.
        */
        struct point xy_to_latlng(point *dest){
            double x = reverseDistanceBetween(std::make_tuple(start.lat_x, start.lng_y), std::make_tuple(dest->lat_x, start.lng_y));
            double y = reverseDistanceBetween(std::make_tuple(start.lat_x, start.lng_y), std::make_tuple(start.lat_x, dest->lng_y));

            point xyz;
            xyz.lat_x = x,
            xyz.lng_y = y,
            xyz.alt_z = start.alt_z;

            return xyz;
        }
        double reverseDistanceBetween(std::tuple<double,double> from, std::tuple<double,double> to){
            return toDegrees(distanceRadians(std::get<0>(from) / EARTH_RADIUS, std::get<1>(from) / EARTH_RADIUS, std::get<0>(to) / EARTH_RADIUS, std::get<1>(to) / EARTH_RADIUS));
        }
        // Radians to Degrees
        double toDegrees(double x){
            return x * (180 / M_PI);
        }

        /*
        * point(lat,lng,alt) -> point(x,y,z)
        * Function that tkes GPSCoord and returns XYZCoord.
        * Will only return positive XY meter distance after distance calculations as angles cannot be uniquely converted.
        * Does not calculate altitude change conversion; returned point will retain starting point altitude.
        */
        struct point latlng_to_xy(point *dest){

            // Compute the distance between the start and destination point's latitudes and longitudes.
            double x = computeDistanceBetween(std::make_tuple(start.lat_x, start.lng_y), std::make_tuple(dest->lat_x, start.lng_y));
            double y = computeDistanceBetween(std::make_tuple(start.lat_x, start.lng_y), std::make_tuple(start.lat_x, dest->lng_y));

            point xyz;
            xyz.lat_x = x,
            xyz.lng_y = y,
            xyz.alt_z = start.alt_z;

            return xyz;
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
        * Haversine formula.
        * See http://www.movable-type.co.uk/scripts/latlong.html
        */
        double hav(double x) {
            return sin(x * 0.5) * sin(x * 0.5);
        }
        /*
        * Inverse haversine formula.
        */
        double arcHav(double x) {
            return 2 * asin(sqrt(x));
        }
        /* 
        * Haversine distance between two points on the unit square.
        */
        double havDistance(double lat1, double lat2, double dLng) {
            return hav(lat1 - lat2) + hav(dLng) * cos(lat1) * cos(lat2);
        }

};

#endif // UTILITIES_CARTESIAN_HPP_