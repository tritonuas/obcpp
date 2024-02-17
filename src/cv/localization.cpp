#include "cv/localization.hpp"
#include <cmath>

#define PI 3.14159265
#define PIXEL_SIZE_MM 0.0024
#define FOCAL_LENGTH_MM 50

// GPS - Functions that take this input this assume the reference ellipsoid defined by WGS84.
struct GPSCoordinates {
    double lat; //Lattitude in radians
    double lon; //Longitude in radians
    double alt; //Altitude in meters
};

// ECEF - Earth Centered, Earth Fixed coordinate system. 0,0,0 is the center of the Earth.
struct ECEFCoordinates {
    double x; //Meters in the plane of the equator in the direction of the prime meridian
    double y; //Meters in the plane of the equator in the direction of 90 degrees East
    double z; //Meters in the direction of the North pole
};

// ENU - East, North, Up coordinate system. Used to show an offset from a certain location on the Earth.
struct ENUCoordinates {
    double e; //Meters East from reference location
    double n; //Meters North from reference location
    double u; //Meters Up from reference location
};

struct CameraIntrinsics {
    double pixelSize;   //mm
    double focalLength; //mm
    double resolutionX; //Pixels
    double resolutionY; //Pixels
};

struct CameraVector {
    double roll;    //Radians
    double pitch;   //Radians
    double heading; //Radians
};

ECEFCoordinates GPStoECEF(GPSCoordinates gps) {
    double a = 6378137;  //Earth semi-major axis in meters
    double b = 6356752;  //Earth semi-minor axis in meters
    double e2 = 1 - (b*b)/(a*a);
    ECEFCoordinates ecef;
    ecef.x = (gps.alt + a/(sqrt(1-e2*sin(gps.lat)*sin(gps.lat))))*cos(gps.lat)*cos(gps.lon);
    ecef.y = (gps.alt + a/(sqrt(1-e2*sin(gps.lat)*sin(gps.lat))))*cos(gps.lat)*sin(gps.lon);
    ecef.z = (gps.alt + (1-e2)*a/(sqrt(1-e2*sin(gps.lat)*sin(gps.lat))))*sin(gps.lat);
    return ecef;
}

// Converts a GPS location and ENU offset to ECEF coordinates
ECEFCoordinates ENUtoECEF(ENUCoordinates offset, GPSCoordinates originGPS) {
    ECEFCoordinates origin = GPStoECEF(originGPS);
    ECEFCoordinates target;
    target.x = origin.x - sin(originGPS.lon)*offset.e - sin(originGPS.lat)*cos(originGPS.lon)*offset.n + cos(originGPS.lat)*cos(originGPS.lon)*offset.u;
    target.y = origin.y + cos(originGPS.lon)*offset.e - sin(originGPS.lat)*sin(originGPS.lon)*offset.n + cos(originGPS.lat)*sin(originGPS.lon)*offset.u;
    target.z = origin.z + cos(originGPS.lat)*offset.n + sin(originGPS.lat)*offset.u;
    return target;
}

// Converts ECEF coordinates to GPS coordinates using Heikkinen's procedure
GPSCoordinates ECEFtoGPS(ECEFCoordinates ecef) {
    GPSCoordinates gps;
    double a = 6378137;
    double b = 6356752;
    double e2 = 1 - ((b*b)/(a*a));
    double ep2 = ((a*a)/(b*b)) - 1;
    double p = sqrt((ecef.x*ecef.x) + (ecef.y*ecef.y));
    double F = 54*(b*b)*(ecef.z*ecef.z);
    double G = (p*p) + ((1 - e2)*(ecef.z*ecef.z)) - (e2*(a*a - b*b));
    double c = e2*e2*F*p*p/(G*G*G);
    double s = cbrt(1 + c + sqrt((c*c) + (2*c)));
    double k = s + 1 + (1/s);
    double P = F/(3*k*k*G*G);
    double Q = sqrt(1 + (2*e2*e2*P));
    double r0 = (-P*e2*p/(1 + Q)) + sqrt((0.5*a*a*(1 + (1/Q))) - (P*(1 - e2)*ecef.z*ecef.z/(Q*(1 + Q))) - (0.5*P*p*p));
    double U = sqrt((p - (e2*r0))*(p - (e2*r0)) + (ecef.z*ecef.z));
    double V = sqrt((p - (e2*r0))*(p - (e2*r0)) + ((1 - e2)*ecef.z*ecef.z));
    double z0 = b*b*ecef.z/(a*V);
    gps.lat = atan((ecef.z + ep2*z0)/p); 
    gps.lon = atan2(ecef.y, ecef.x);
    gps.alt = U*(1 - ((b*b)/(a*V)));
    return gps;
}

// Calculate angle offset based on target pixel coordinates using pinhole camera model
CameraVector PixelsToAngle(CameraIntrinsics camera, CameraVector state, double targetX, double targetY) {
    CameraVector target;
    target.roll = atan(camera.pixelSize*(targetX - (camera.resolutionX/2))/camera.focalLength);
    target.pitch = atan(camera.pixelSize*(targetY - (camera.resolutionY/2))/camera.focalLength);
    target.heading = state.heading;
    return target;
}

// Calculate the ENU offset of the intersection of a vector from the plane to the ground (assume flat)
ENUCoordinates AngleToENU(CameraVector target, GPSCoordinates aircraft, double terrainHeight) {
    double x = aircraft.alt*tan(target.roll);
    double y = aircraft.alt*tan(target.pitch); 
    ENUCoordinates offset;
    offset.e = x*cos(target.heading) + y*sin(target.heading);
    offset.n = -x*sin(target.heading) + y*cos(target.heading);
    offset.u = terrainHeight - aircraft.alt;
    return offset;
}

GPSCoord Localization::localize(const ImageTelemetry& telemetry, const Bbox& targetBbox) {
    double terrainHeight = 0;

    double targetX = (targetBbox.x1 + targetBbox.x2)/2;
    double targetY = (targetBbox.y1 + targetBbox.y2)/2;

    CameraIntrinsics camera;
    camera.pixelSize = PIXEL_SIZE_MM;
    camera.focalLength = FOCAL_LENGTH_MM;
    camera.resolutionX = 5472;
    camera.resolutionY = 3648;

    CameraVector gimbalState;
    gimbalState.roll = telemetry.roll*PI/180;
    gimbalState.pitch = telemetry.pitch*PI/180;
    gimbalState.heading = telemetry.yaw*PI/180;

    GPSCoordinates aircraft;
    aircraft.lat = telemetry.latitude*PI/180;
    aircraft.lon = telemetry.longitude*PI/180;
    aircraft.alt = telemetry.altitude*1000;

    CameraVector targetVector = PixelsToAngle(camera, gimbalState, targetX, targetY);
    ENUCoordinates offset = AngleToENU(targetVector, aircraft, terrainHeight);
    ECEFCoordinates targetLocationECEF = ENUtoECEF(offset, aircraft);
    GPSCoordinates targetLocationGPS = ECEFtoGPS(targetLocationECEF);

    double lat = targetLocationGPS.lat*180/PI;
    double lon = targetLocationGPS.lon*180/PI;
    double alt = targetLocationGPS.alt/1000;

    GPSCoord targetCoord;
    targetCoord.set_latitude(lat);
    targetCoord.set_longitude(lon);
    targetCoord.set_altitude(alt);
    return targetCoord;
}
