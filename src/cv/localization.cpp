#include "cv/localization.hpp"
#include <cmath>

#define PI 3.14159265

// struct ECEFCoordinates;
// struct CameraVector;
// struct ENUCoordinates;

ECEFLocalization::ECEFCoordinates ECEFLocalization::GPStoECEF(GPSCoord gps) {
    double a = 6378137;  // Earth semi-major axis in meters
    double b = 6356752;  // Earth semi-minor axis in meters
    double e2 = 1 - (b*b)/(a*a);
    ECEFCoordinates ecef;
    ecef.x = (gps.altitude() + a/
        (sqrt(1-e2*sin(gps.latitude())*sin(gps.latitude()))))*
        cos(gps.latitude())*cos(gps.longitude());
    ecef.y = (gps.altitude() + a/
        (sqrt(1-e2*sin(gps.latitude())*sin(gps.latitude()))))*
        cos(gps.latitude())*sin(gps.longitude());
    ecef.z = (gps.altitude() + (1-e2)*a/
        (sqrt(1-e2*sin(gps.latitude())*sin(gps.latitude()))))*sin(gps.latitude());
    return ecef;
}

// Converts a GPS location and ENU offset to ECEF coordinates
ECEFLocalization::ECEFCoordinates ECEFLocalization::ENUtoECEF(
    ENUCoordinates offset, GPSCoord originGPS) {
    ECEFCoordinates origin = GPStoECEF(originGPS);
    ECEFCoordinates target;
    target.x = origin.x - sin(originGPS.longitude())*offset.e -
        sin(originGPS.latitude())*cos(originGPS.longitude())*offset.n +
        cos(originGPS.latitude())*cos(originGPS.longitude())*offset.u;
    target.y = origin.y + cos(originGPS.longitude())*offset.e -
        sin(originGPS.latitude())*sin(originGPS.longitude())*offset.n +
        cos(originGPS.latitude())*sin(originGPS.longitude())*offset.u;
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
    double r0 = (-P*e2*p/(1 + Q)) + sqrt((0.5*a*a*(1 + (1/Q))) -
        (P*(1 - e2)*ecef.z*ecef.z/(Q*(1 + Q))) - (0.5*P*p*p));
    double U = sqrt((p - (e2*r0))*(p - (e2*r0)) + (ecef.z*ecef.z));
    double V = sqrt((p - (e2*r0))*(p - (e2*r0)) + ((1 - e2)*ecef.z*ecef.z));
    double z0 = b*b*ecef.z/(a*V);
    gps.set_latitude(atan((ecef.z + ep2*z0)/p));
    gps.set_longitude(atan2(ecef.y, ecef.x));
    gps.set_altitude(U*(1 - ((b*b)/(a*V))));
    return gps;
}

// Calculate angle offset based on target pixel coordinates using pinhole camera model
ECEFLocalization::CameraVector ECEFLocalization::PixelsToAngle(
    CameraIntrinsics camera, CameraVector state, double targetX, double targetY) {
    CameraVector target;
    target.roll = atan(camera.pixelSize*(targetX - (camera.resolutionX/2))/camera.focalLength);
    target.pitch = atan(camera.pixelSize*(targetY - (camera.resolutionY/2))/camera.focalLength);
    target.heading = state.heading;
    return target;
}

// Calculate the ENU offset of the intersection of a vector from
// the plane to the ground (assume flat)
ECEFLocalization::ENUCoordinates ECEFLocalization::AngleToENU(
    CameraVector target, GPSCoord aircraft, double terrainHeight) {
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
    gimbalState.roll = telemetry.roll_deg*PI/180;
    gimbalState.pitch = telemetry.pitch_deg*PI/180;
    gimbalState.heading = telemetry.yaw_deg*PI/180;

    GPSCoord aircraft;
    aircraft.set_latitude(telemetry.latitude_deg*PI/180);
    aircraft.set_longitude(telemetry.longitude_deg*PI/180);
    aircraft.set_altitude(telemetry.altitude_agl_m*1000);

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
    // double GSD = (SENSOR_WIDTH * (telemetry.altitude_agl_m * 1000))
    //              / (FOCAL_LENGTH_MM * IMG_WIDTH_PX);

    double GSD = (ANGLE_OF_VIEW_RATIO * (telemetry.altitude_agl_m * 1000)
                 /IMG_WIDTH_PX);

    // Midpoints of the image
    double img_mid_x = IMG_WIDTH_PX / 2;
    double img_mid_y = IMG_HEIGHT_PX / 2;

    // midpoints of bounding box around the target
    double target_x = (targetBbox.x1 + targetBbox.x2)/2;
    double target_y = (targetBbox.y1 + targetBbox.y2)/2;

    // calculations of bearing
    // L = (distance(middle, bbox))*GSD
    double length = (sqrt(pow((target_x - img_mid_x), 2) + pow((target_y - img_mid_y), 2) * GSD));

    // Translate Image Cordinates to Camera Cordinate (Origin to Center of Image instead of Top Left) NOLINT
    double target_camera_cord_x = target_x - (IMG_WIDTH_PX / 2);
    double target_camera_cord_y = (IMG_HEIGHT_PX / 2) - target_y;

    // Convert to polar coordinates
    double target_camera_cord_r = sqrt((target_camera_cord_y * target_camera_cord_y)
    + (target_camera_cord_x * target_camera_cord_x));
    double target_camera_cord_theta;

    // Check if xy coord is in quadrant 2 or 3, if so need to add pi
    // (atan returns a value in the range -π/2 to π/2 radians)
    // also check for if x coord == 0, if so just set theta to pi or -pi to
    // (avoid divison by 0 in the atan function)
    if (target_camera_cord_x < 0 && target_camera_cord_y < 0) {
        target_camera_cord_theta = atan(target_camera_cord_y/target_camera_cord_x) + M_PI;

    } else if (target_camera_cord_x < 0 && target_camera_cord_y > 0) {
        target_camera_cord_theta = atan(target_camera_cord_y/target_camera_cord_x) + M_PI;

    } else if (target_camera_cord_x == 0) {
        if (target_camera_cord_y > 1) {
            target_camera_cord_theta == M_PI;
        } else {
            target_camera_cord_theta == -M_PI;
        }

    } else {
        target_camera_cord_theta = atan(target_camera_cord_y/target_camera_cord_x);
    }

    // Transfrom the coordinate to real-world orientation by subtracting heading angle
    double hdg_radians = telemetry.heading_deg * M_PI / 180;
    target_camera_cord_theta = target_camera_cord_theta - hdg_radians;

    // Convert back to regular coordinates
    target_camera_cord_x = target_camera_cord_r*cos(target_camera_cord_theta);
    target_camera_cord_y = target_camera_cord_r*sin(target_camera_cord_theta);

    // Finds the offset of the bbox
    double calc_cam_offset_x_m = target_camera_cord_x * GSD * 0.001;  // mm to M
    double calc_cam_offset_y_m = target_camera_cord_y * GSD * 0.001;  // mm to M

    // Calculates the cordinates using the offset
    GPSCoord calc_coord = CalcOffset((calc_cam_offset_x_m), (calc_cam_offset_y_m),
                                    (telemetry.latitude_deg), (telemetry.longitude_deg));

    return calc_coord;
}

/*
Takes the in two cordinaates and outputs their distance in meters. 

Parameters:
- lat1/lon1 (First Cordinate)
- lat2/lon2 (Second Cordinate)

@returns distance in meters

Reference: http://www.movable-type.co.uk/scripts/latlong.html
*/

double GSDLocalization::distanceInMetersBetweenCords(const double lat1, const double lon1, const double lat2, const double lon2) { // NOLINT
    double e1 = lat1 * M_PI / 180;
    double e2 = lat2 * M_PI / 180;

    double d1 = (lat2 - lat1) * M_PI / 180;
    double d2 = (lon2 - lon1) * M_PI / 180;

    double a = sin(d1/2) * sin(d1/2) + cos(e1) * cos(e2) * sin(d2/2) * sin(d2/2);

    double c = 2 * atan2(sqrt(a), sqrt(1-a));

    double d = EARTH_RADIUS_M * c;

    return d;
}

/*
Takes the position of the camera in blender and the position of the generated target in meters

Parameters:
-image_offset_x/y - meters from center of plane (0,0)
-cam_lat/lon - Set cordinates of plane

@returns true (mostly) world cordinate of target 
*/

GPSCoord GSDLocalization::CalcOffset(const double offset_x, const double offset_y, const double lat, const double lon) { // NOLINT
    double dLat = offset_y / EARTH_RADIUS_M;
    double dLon = offset_x / (EARTH_RADIUS_M * cos(M_PI * lat / 180));

    double latO = lat + dLat * 180/M_PI;
    double lonO = lon + dLon * 180/M_PI;

    GPSCoord output;

    output.set_latitude(latO);
    output.set_longitude(lonO);

    return output;
}

std::tuple<double, double, double>
GSDLocalization::debug(const ImageTelemetry& telemetry, const Bbox& targetBbox) {
    GPSCoord gps;

    // Ground Sample Distance (mm/pixel), 1.0~2.5cm per px is ideal aka 10mm~25mm ppx
    // double GSD = (SENSOR_WIDTH * (telemetry.altitude_agl_m * 1000))
    //              / (FOCAL_LENGTH_MM * IMG_WIDTH_PX);

    double GSD = (ANGLE_OF_VIEW_RATIO * (telemetry.altitude_agl_m * 1000)
                 /IMG_WIDTH_PX);

    // Midpoints of the image
    double img_mid_x = IMG_WIDTH_PX / 2;
    double img_mid_y = IMG_HEIGHT_PX / 2;

    // midpoints of bounding box around the target
    double target_x = (targetBbox.x1 + targetBbox.x2)/2;
    double target_y = (targetBbox.y1 + targetBbox.y2)/2;

    // calculations of bearing
    // L = (distance(middle, bbox))*GSD
    double length = (sqrt(pow((target_x - img_mid_x), 2) + pow((target_y - img_mid_y), 2) * GSD));

    // Translate Image Cordinates to Camera Cordinate (Origin to Center of Image instead of Top Left) NOLINT
    double target_camera_cord_x = target_x - (IMG_WIDTH_PX / 2);
    double target_camera_cord_y = (IMG_HEIGHT_PX / 2) - target_y;

    // Convert to polar coordinates
    double target_camera_cord_r =
    sqrt((target_camera_cord_y * target_camera_cord_y)
    + (target_camera_cord_x * target_camera_cord_x));
    double target_camera_cord_theta;

    // Check if xy coord is in quadrant 2 or 3,
    // f so need to add pi (atan returns a value in the range -π/2 to π/2 radians)
    // also check for if x coord == 0,
    // if so just set theta to pi or -pi to avoid divison by 0 in the atan function
    if (target_camera_cord_x < 0 && target_camera_cord_y < 0) {
        target_camera_cord_theta = atan(target_camera_cord_y/target_camera_cord_x) + M_PI;

    } else if (target_camera_cord_x < 0 && target_camera_cord_y > 0) {
        target_camera_cord_theta = atan(target_camera_cord_y/target_camera_cord_x) + M_PI;

    } else if (target_camera_cord_x == 0) {
        if (target_camera_cord_y > 1) {
            target_camera_cord_theta == M_PI;
        } else {
            target_camera_cord_theta == -M_PI;
        }
    } else {
        target_camera_cord_theta = atan(target_camera_cord_y/target_camera_cord_x);
    }

    // Transfrom the coordinate to real-world orientation by subtracting heading angle
    double hdg_radians = (telemetry.heading_deg) * M_PI / 180;
    target_camera_cord_theta = target_camera_cord_theta - hdg_radians;
    // Convert back to regular coordinates
    target_camera_cord_x = target_camera_cord_r*cos(target_camera_cord_theta);
    target_camera_cord_y = target_camera_cord_r*sin(target_camera_cord_theta);
    // Finds the offset of the bbox
    double calc_cam_offset_x_m = target_camera_cord_x * GSD * 0.001;  // mm to M
    double calc_cam_offset_y_m = target_camera_cord_y * GSD * 0.001;  // mm to M
    return std::make_tuple(GSD, calc_cam_offset_x_m, calc_cam_offset_y_m);
}
