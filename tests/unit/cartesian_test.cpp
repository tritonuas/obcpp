#include <gtest/gtest.h>

#include <vector>

#include "pathing/cartesian.hpp"
#include "protos/obc.pb.h"
#include "utilities/datatypes.hpp"

const std::vector<GPSCoord> BOUNDS = {
    makeGPSCoord(51.022393690441405, 9.970974722308316, 0),
    makeGPSCoord(50.98521082089737, 9.969772758406188, 0),
    makeGPSCoord(50.98510343613449, 10.037109701308127, 0),
    makeGPSCoord(51.022601441548915, 10.033330587165663, 0)
};

CartesianConverter<std::vector<GPSCoord>> c(BOUNDS);

// Make sure conversions are inverses of eachother
TEST(CartesianTest, IdentityMaps) {
    GPSCoord c1 = makeGPSCoord(51.022393690441405, 9.970974722308316, 150);
    GPSCoord c2 = c.toLatLng(c.toXYZ(c1));

    EXPECT_DOUBLE_EQ(c1.latitude(), c2.latitude());
    EXPECT_DOUBLE_EQ(c1.longitude(), c2.longitude());
    EXPECT_DOUBLE_EQ(c1.altitude(), c2.altitude());
}

// Make sure the center is mapping to 0
TEST(CartesianTest, CenterMapsToZero) {
    GPSCoord center = c.getCenter();

    XYZCoord zero = c.toXYZ(center);
    EXPECT_DOUBLE_EQ(0.0, zero.x);
    EXPECT_DOUBLE_EQ(0.0, zero.y);
    EXPECT_DOUBLE_EQ(0.0, zero.z);
}

// TODO: more tests