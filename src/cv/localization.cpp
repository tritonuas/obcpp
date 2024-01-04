#include "cv/localization.hpp"

GPSCoord Localization::localize(const ImageTelemetry& telemetry,
    const Bbox& targetBbox) {

    return GPSCoord(0, 0, 0);
}