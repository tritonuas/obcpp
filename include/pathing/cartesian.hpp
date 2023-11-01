#ifndef UTILITIES_CARTESIAN_HPP_
#define UTILITIES_CARTESIAN_HPP_
#define _USE_MATH_DEFINES
#include <algorithm>
#include <math.h>
#include <string>
#include "../utilities/datatypes.hpp"

/*
* Converter for between (lat,lon,alt) and (x,y,z).
*/
class CartesianConverter {
    private:
        GPSCoord startGPS;
        XYZCoord startXYZ;
        double EARTH_RADIUS = 6371008.7714; // Mean radius of the Earth as approximated by the IUGG.

    public:
        CartesianConverter(){};
        CartesianConverter(GPSCoord *pt){
            startGPS = *pt;
        };
        CartesianConverter(XYZCoord *pt){
            startXYZ = *pt;
        };
        CartesianConverter(GPSCoord *GPSpt, XYZCoord *XYZpt){
            startGPS = *GPSpt;
            startXYZ = *XYZpt;
        }

        /*
        * XYZCoord(x,y,z) -> GPSCoord(lat,lon,alt)
        * Function that takes XYZCoord and returns GPSCoord.
        * Requires a CartesianConverter with a starting XYZCoord.
        * Will return only positive coordinates after distance calculations as angles cannot be uniquely converted.
        * Does not calculate altitude change conversion; returned GPSCoord will retain starting GPSCoord altitude.
        */
        struct GPSCoord xy_to_gps(XYZCoord *dest){
            
            // Compute the distance between start and destination XYZCoord.
            double lat = reverseDistanceBetween(std::make_tuple(startXYZ.x, startXYZ.y), std::make_tuple(dest->x, startXYZ.y));
            double lon = reverseDistanceBetween(std::make_tuple(startXYZ.x, startXYZ.y), std::make_tuple(startXYZ.x, dest->y));

            GPSCoord gps(lat,lon,startXYZ.y);    

            return gps;
        }
        double reverseDistanceBetween(std::tuple<double,double> from, std::tuple<double,double> to){
            return toDegrees(distanceRadians(std::get<0>(from) / EARTH_RADIUS, std::get<1>(from) / EARTH_RADIUS, std::get<0>(to) / EARTH_RADIUS, std::get<1>(to) / EARTH_RADIUS));
        }
        // Radians to Degrees
        double toDegrees(double x){
            return x * (180 / M_PI);
        }

        /*
        * GPSCoord(lat,lon,alt) -> XYZCoord(x,y,z)
        * Function that tkes GPSCoord and returns XYZCoord.
        * Requires a CartesianConverter with a starting GPSCoord.
        * Will only return positive XY meter distance after distance calculations as angles cannot be uniquely converted.
        * Does not calculate altitude change conversion; returned GPSCoord will retain starting GPSCoord altitude.
        */
        struct XYZCoord gps_to_xy(GPSCoord *dest){

            // Compute the distance between start and destination GPSCoord.
            double x = computeDistanceBetween(std::make_tuple(startGPS.lat, startGPS.lon), std::make_tuple(dest->lat, startGPS.lon));
            double y = computeDistanceBetween(std::make_tuple(startGPS.lat, startGPS.lon), std::make_tuple(startGPS.lat, dest->lon));

            XYZCoord xyz(x,y,startGPS.alt);

            return xyz;
        }

        /* 
        * Calculate final distance between two tuple (latitude, longitude).
        */
        double computeDistanceBetween(std::tuple<double,double> from, std::tuple<double,double> to){
            return computeAngleBetween(from, to) * EARTH_RADIUS;
        }
        /*
        * Calculate radian angle on the unit circle between two tuple (latitude, longitude).
        */
        double computeAngleBetween(std::tuple<double,double> from, std::tuple<double,double> to){
            return distanceRadians(toRadians(std::get<0>(from)), toRadians(std::get<1>(from)), toRadians (std::get<0>(to)), toRadians(std::get<1>(to)));
        }
        /*
        * Calculate radian distance on the unit circle between two (radian, radian) on the unit circle.
        */
        double distanceRadians(double lat1, double lon1, double lat2, double lon2) {
            return arcHav(havDistance(lat1, lat2, lon1 - lon2));
        }

        /*
        * Degrees to radians.
        */
        double toRadians(double x){
            return x * (M_PI / 180);
        }
        /*
        * Haversine formula.
        * See http://www.movable-type.co.uk/scripts/gpsg.html
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

        /*
        * Getters and setters
        */
       GPSCoord get_GPS(){
        return startGPS;
       }
       XYZCoord get_XYZ(){
        return startXYZ;
       }
       void set_GPS(GPSCoord *pt){
        startGPS = *pt;
       }
       void set_XYZ(XYZCoord *pt){
        startXYZ = *pt;
       }
};

#endif // UTILITIES_CARTESIAN_HPP_