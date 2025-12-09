#include <gtest/gtest.h>
#include "utilities/jsonable.hpp"
#include "utilities/datatypes.hpp"

TEST(XYZCoordJSON, jsonable_test) {

    const double x = 1.0;
    const double y = 2.0;
    const double z = 3.0;

    XYZCoord xyz_coord_obj(x, y, z);

    json output = xyz_coord_obj.to_json();
    
    EXPECT_EQ(x, output["x"]);
    EXPECT_EQ(y, output["y"]);
    EXPECT_EQ(z, output["z"]);
}