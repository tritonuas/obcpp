#ifndef INCLUDE_CV_LOCALIZATION_HPP_
#define INCLUDE_CV_LOCALIZATION_HPP_

#include "cv/utilities.hpp"

#include "camera/interface.hpp"
#include "utilities/datatypes.hpp"

// TODO: these should be constants in the config file
// (or maybe queried by camera)
#define PIXEL_SIZE_MM 0.0024
#define FOCAL_LENGTH_MM 50
#define IMG_WIDTH_PX 5472
#define IMG_HEIGHT_PX 3648
#define EARTH_RADIUS_M 6378137
#define SENSOR_WIDTH 15.86  // mm
#define METER_TO_FT 3.28084

// Localization is responsible for calculating the real world latitude/longitude
// of competition targets.
// See our Python implementation here: https://github.com/tritonuas/localization
class Localization {
 public:
    // localize is responsible for transforming the position of a target
    // within a full resolution image (image coordinates) to it's position
    // in the real world (latitude/longitude coords). We are given the
    // pixel coordinates of the target from saliency and the GPS position of
    // the plane at the time of image capture.
    //
    // TODO: also need to pass in camera/lens information such as sensor width,
    // focal length, and image width/height
    virtual GPSCoord localize(const ImageTelemetry& telemetry, const Bbox& targetBbox) = 0;

 protected:
    struct CameraIntrinsics {
        double pixelSize;    // mm
        double focalLength;  // mm
        double resolutionX;  // Pixels
        double resolutionY;  // Pixels
    };

    CameraIntrinsics camera{
        .pixelSize = PIXEL_SIZE_MM,
        .focalLength = FOCAL_LENGTH_MM,
        .resolutionX = IMG_WIDTH_PX,
        .resolutionY = IMG_HEIGHT_PX,
    };
};

// Localization by converting via ECEF coordinates (Earth Centered, Earth Fixed)
class ECEFLocalization : Localization {
 public:
    GPSCoord localize(const ImageTelemetry& telemetry, const Bbox& targetBbox) override;

 private:
    // ECEF - Earth Centered, Earth Fixed coordinate system. 0,0,0 is the center of the Earth.
    struct ECEFCoordinates {
        double x;  // Meters in the plane of the equator in the direction of the prime meridian
        double y;  // Meters in the plane of the equator in the direction of 90 degrees East
        double z;  // Meters in the direction of the North pole
    };

    // ENU - East, North, Up coordinate system.
    // Used to show an offset from a certain location on the Earth.
    struct ENUCoordinates {
        double e;  // Meters East from reference location
        double n;  // Meters North from reference location
        double u;  // Meters Up from reference location
    };


    struct CameraVector {
        double roll;     // Radians
        double pitch;    // Radians
        double heading;  // Radians
    };

    ECEFCoordinates GPStoECEF(GPSCoord gps);
    ECEFCoordinates ENUtoECEF(ENUCoordinates offset, GPSCoord originGPS);
    GPSCoord ECEFtoGPS(ECEFCoordinates ecef);
    CameraVector PixelsToAngle(
        CameraIntrinsics camera,
        CameraVector state,
        double targetX,
        double targetY);
    ENUCoordinates AngleToENU(CameraVector target, GPSCoord aircraft, double terrainHeight);
};

// Localization using GSD (ground sample distance) ratio
class GSDLocalization : Localization {
 public:
    GPSCoord localize(const ImageTelemetry& telemetry, const Bbox& targetBbox) override;
    GPSCoord CalcOffset(const double offset_x, const double offset_y,
                        const double lat, const double lon);
    double distanceInMetersBetweenCords(const double lat1, const double lon1,
                                        const double lat2, const double lon2);
};

#endif  // INCLUDE_CV_LOCALIZATION_HPP_
