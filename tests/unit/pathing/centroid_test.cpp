
#include <gtest/gtest.h>
#include "pathing/environment.hpp"
#include <iostream>

TEST(Centroid, Square) {
    std::vector<XYZCoord> contour {
        {0.0, 0.0, 0.0},
        {10.0, 0.0, 0.0},
        {10.0, 10.0, 0.0}, 
        {0.0, 10.0, 0.0}
    };

    XYZCoord centroid = XYZCoord(5.0, 5.0, 0.0);
    Environment env = Environment(contour, {}, {}, {}, {});
    XYZCoord predCentroid = env.findCentroid();

    std::cout << centroid.x << '\n';
    std::cout << predCentroid.x;

    EXPECT_EQ(centroid.x, predCentroid.x);
    EXPECT_EQ(centroid.y, predCentroid.y);
    EXPECT_EQ(centroid.z, predCentroid.z);
}