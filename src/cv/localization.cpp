#include "cv/localization.hpp"
#include <cmath>

#define PI 3.14159265

// struct ECEFCoordinates;
// struct CameraVector;
// struct ENUCoordinates;

ECEFLocalization::ECEFCoordinates ECEFLocalization::GPStoECEF(GPSCoord gps) {
    double a = 6378137;  //Earth semi-major axis in meters
    double b = 6356752;  //Earth semi-minor axis in meters
    double e2 = 1 - (b*b)/(a*a);
    ECEFCoordinates ecef;
    ecef.x = (gps.altitude() + a/(sqrt(1-e2*sin(gps.latitude())*sin(gps.latitude()))))*cos(gps.latitude())*cos(gps.longitude());
    ecef.y = (gps.altitude() + a/(sqrt(1-e2*sin(gps.latitude())*sin(gps.latitude()))))*cos(gps.latitude())*sin(gps.longitude());
    ecef.z = (gps.altitude() + (1-e2)*a/(sqrt(1-e2*sin(gps.latitude())*sin(gps.latitude()))))*sin(gps.latitude());
    return ecef;
}

// Converts a GPS location and ENU offset to ECEF coordinates
ECEFLocalization::ECEFCoordinates ECEFLocalization::ENUtoECEF(ENUCoordinates offset, GPSCoord originGPS) {
    ECEFCoordinates origin = GPStoECEF(originGPS);
    ECEFCoordinates target;
    target.x = origin.x - sin(originGPS.longitude())*offset.e - sin(originGPS.latitude())*cos(originGPS.longitude())*offset.n + cos(originGPS.latitude())*cos(originGPS.longitude())*offset.u;
    target.y = origin.y + cos(originGPS.longitude())*offset.e - sin(originGPS.latitude())*sin(originGPS.longitude())*offset.n + cos(originGPS.latitude())*sin(originGPS.longitude())*offset.u;
    target.z = origin.z + cos(originGPS.longitude())*offset.n + sin(originGPS.latitude())*offset.u;
    return target;
}

// Converts ECEF coordinates to GPS coordinates using Heikkinen's procedure
GPSCoord ECEFLocalization::ECEFtoGPS(ECEFCoordinates ecef) {
    GPSCoord gps;
    double a = EARTH_RADIUS_METERS;
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
    gps.set_latitude(atan((ecef.z + ep2*z0)/p)); 
    gps.set_longitude(atan2(ecef.y, ecef.x));
    gps.set_altitude(U*(1 - ((b*b)/(a*V))));
    return gps;
}

// Calculate angle offset based on target pixel coordinates using pinhole camera model
ECEFLocalization::CameraVector ECEFLocalization::PixelsToAngle(CameraIntrinsics camera, CameraVector state, double targetX, double targetY) {
    CameraVector target;
    target.roll = atan(camera.pixelSize*(targetX - (camera.resolutionX/2))/camera.focalLength);
    target.pitch = atan(camera.pixelSize*(targetY - (camera.resolutionY/2))/camera.focalLength);
    target.heading = state.heading;
    return target;
}

// Calculate the ENU offset of the intersection of a vector from the plane to the ground (assume flat)
ECEFLocalization::ENUCoordinates ECEFLocalization::AngleToENU(CameraVector target, GPSCoord aircraft, double terrainHeight) {
    double x = aircraft.altitude()*tan(target.roll);
    double y = aircraft.altitude()*tan(target.pitch); 
    ENUCoordinates offset;
    offset.e = x*cos(target.heading) + y*sin(target.heading);
    offset.n = -x*sin(target.heading) + y*cos(target.heading);
    offset.u = terrainHeight - aircraft.altitude();
    return offset;
}

GPSCoord ECEFLocalization::localize(const ImageTelemetry& telemetry, const Bbox& targetBbox) {
    double terrainHeight = 0;

    double targetX = (targetBbox.x1 + targetBbox.x2)/2;
    double targetY = (targetBbox.y1 + targetBbox.y2)/2;


    CameraVector gimbalState;
    gimbalState.roll = telemetry.roll*PI/180;
    gimbalState.pitch = telemetry.pitch*PI/180;
    gimbalState.heading = telemetry.yaw*PI/180;

    GPSCoord aircraft;
    aircraft.set_latitude(telemetry.latitude*PI/180);
    aircraft.set_longitude(telemetry.longitude*PI/180);
    aircraft.set_altitude(telemetry.altitude*1000);

    CameraVector targetVector = PixelsToAngle(camera, gimbalState, targetX, targetY);
    ENUCoordinates offset = AngleToENU(targetVector, aircraft, terrainHeight);
    ECEFCoordinates targetLocationECEF = ENUtoECEF(offset, aircraft);
    GPSCoord targetLocationGPS = ECEFtoGPS(targetLocationECEF);

    double lat = targetLocationGPS.latitude()*180/PI;
    double lon = targetLocationGPS.longitude()*180/PI;
    double alt = targetLocationGPS.altitude()/1000;

    GPSCoord targetCoord;
    targetCoord.set_latitude(lat);
    targetCoord.set_longitude(lon);
    targetCoord.set_altitude(alt);
    return targetCoord;
}


GPSCoord GSDLocalization::localize(const ImageTelemetry& telemetry, const Bbox& targetBbox) {
    GPSCoord gps;

    // Ground Sample Distance (mm/pixel), 1.0~2.5cm per px is ideal aka 10mm~25mm ppx
    float GSD = (SENSOR_WIDTH * (telemetry.altitude)) / (FOCAL_LENGTH_MM * IMG_WIDTH_PX);

    // Midpoints of the image
    float img_mid_x = IMG_WIDTH_PX / 2;
    float img_mid_y = IMG_HEIGHT_PX / 2;

    //midpoints of bounding box around the target
    float target_x = (targetBbox.x1 + targetBbox.x2)/2;
    float target_y = (targetBbox.y1 + targetBbox.y2)/2;

    // calculations of bearing
    // L = (distance(middle, bbox))*GSD  
    float length = (sqrt(pow((target_x - img_mid_x), 2) + pow((target_y - img_mid_y), 2) * GSD));

    //Translate Image Cordinates to Camera Cordinate Frame (Origin to Center of Image instead of Top Left)
    float target_camera_cord_x = target_x - (IMG_WIDTH_PX / 2);
    float target_camera_cord_y = (IMG_HEIGHT_PX / 2) - target_y;

    //Angle of Bearing (Angle from north to target)
    float thetaB = plane_data[3] + atan(target_camera_cord_x / target_camera_cord_y);

    //Translate bearing to the 3 quadrant if applicable
    if (target_camera_cord_x < 0 && target_camera_cord_y < 0){
        thetaB = 180.0 + thetaB;
    }

    //Finds the offset of the bbox 
    float calc_cam_offset_x = target_camera_cord_x * GSD * 0.001; //mm to M
    float calc_cam_offset_y = target_camera_cord_y * GSD * 0.001; //mm to M

    //Calculates the cordinates using the offset
    GPSCoord calc_coord = calc_offset(calc_cam_offset_y, calc_cam_offset_x, telemetry.latitude, telemetry.longitude);

    return calc_coord;
}

/*
Takes the position of the camera in blender and the position of the generated target in meters

Parameters:
-image_offset_x/y - meters from center of plane (0,0)
-cam_lat/lon - Set cordinates of plane

@returns true (mostly) world cordinate of target 
*/

GPSCoord GSDLocalization::CalcOffset(const float offset_x, const float offset_y, const float lat, const float lon)  {
    float dLat = offset_y / EARTH_RADIUS_M;
    float dLon = offset_x / (EARTH_RADIUS_M * cos(M_PI * lat / 180));

    float latO = lat + dLat * 180/M_PI;
    float lonO = lon + dLon * 180/M_PI;

    GPDCoord output;

    output.set_latitude(latO);
    output.set_longitude(lonO);

    return output;
}