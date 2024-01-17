#ifndef INCLUDE_CV_LOCALIZATION_HPP_
#define INCLUDE_CV_LOCALIZATION_HPP_

#include "cv/utilities.hpp"

#include "camera/interface.hpp"
#include "utilities/datatypes.hpp"

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
        GPSCoord localize(const ImageTelemetry& telemetry, const Bbox& targetBbox);
};

#endif  // INCLUDE_CV_LOCALIZATION_HPP_
