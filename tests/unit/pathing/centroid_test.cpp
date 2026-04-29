
#include <gtest/gtest.h>
#include "pathing/environment.hpp"
#include <iostream>

TEST(ScaleContour, Square) {
 
    std::vector<XYZCoord> contour {
        {0.0, 0.0, 0.0},
        {10.0, 0.0, 0.0},
        {10.0, 10.0, 0.0}, 
        {0.0, 10.0, 0.0}
    };

    std::vector<XYZCoord> contour_scaled {
        {2.0, 2.0, 0.0},
        {8.0, 2.0, 0.0},
        {8.0, 8.0, 0.0}, 
        {2.0, 8.0, 0.0}
    };
    
    Environment env = Environment({}, {}, {}, {}, {});
    Polygon pred_contour = env.scaleFixedDistance(2, contour);

    for (int i = 0; i < pred_contour.size(); ++i) {
        EXPECT_EQ(pred_contour[i].x, contour_scaled[i].x);
        EXPECT_EQ(pred_contour[i].y, contour_scaled[i].y);
    }
}
