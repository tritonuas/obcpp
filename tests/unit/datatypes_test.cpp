#include "utilities/datatypes.hpp"

#include <gtest/gtest.h>

#include <cmath>
/*
 *   Way the tests are written is non-standarzied
 *   becuase I'm lazy
 */

/*
 *   tests XYZCoord::==
 */
TEST(XYZCoordOperations, equals) {
  XYZCoord origin{0, 0, 0};
  EXPECT_EQ(origin == (XYZCoord{0, 0, 0}), true);
  EXPECT_EQ(origin == (XYZCoord{1, 0, 0}), false);
  EXPECT_EQ(origin == (XYZCoord{0, 1, 0}), false);
  EXPECT_EQ(origin == (XYZCoord{0, 0, 1}), false);
  EXPECT_EQ(origin == (XYZCoord{1, 1, 0}), false);

  // make sure there is no double weirdness
  XYZCoord rand_point{0.1, -0.1, 1683};
  EXPECT_EQ(rand_point == (XYZCoord{0.1, -0.1, 1683}), true);
  EXPECT_EQ(rand_point == (XYZCoord{0, -0.1, 1683}), false);
  EXPECT_EQ(rand_point == (XYZCoord{0.1, 0.1, 1683}), false);
  EXPECT_EQ(rand_point == (XYZCoord{0.1, -0.1, 683}), false);

  EXPECT_EQ(rand_point == origin, false);
}

/*
 *   tests XYZCoord::+=
 */
TEST(XYZCoordOperations, plusEquals) {
  XYZCoord mutated_coord{0, 0, 0};
  EXPECT_EQ(mutated_coord, (XYZCoord{0, 0, 0}));

  mutated_coord += XYZCoord{0, 0, 0};
  EXPECT_EQ(mutated_coord, (XYZCoord{0, 0, 0}));

  mutated_coord += XYZCoord{1, 1, 1};
  EXPECT_EQ(mutated_coord, (XYZCoord{1, 1, 1}));

  mutated_coord += XYZCoord{15.263, -548526.3, 789.63};
  EXPECT_NEAR(mutated_coord.x, 16.263, 0.001);
  EXPECT_NEAR(mutated_coord.y, -548525.3, 0.001);
  EXPECT_NEAR(mutated_coord.z, 790.63, 0.001);
}

/*
 *   tests XYZCoord::+
 */
TEST(XYZCoordOperations, addition) {
  XYZCoord origin{0, 0, 0};
  EXPECT_EQ(origin, (XYZCoord{0, 0, 0}));

  EXPECT_EQ(origin + (XYZCoord{0, 0, 0}), (XYZCoord{0, 0, 0}));

  EXPECT_EQ(origin + (XYZCoord{1, 1, 1}), (XYZCoord{1, 1, 1}));

  XYZCoord arbitrary_point =
      XYZCoord{1, 1, 1} + XYZCoord{15.263, -548526.3, 789.63};
  EXPECT_NEAR(arbitrary_point.x, 16.263, 0.001);
  EXPECT_NEAR(arbitrary_point.y, -548525.3, 0.001);
  EXPECT_NEAR(arbitrary_point.z, 790.63, 0.001);
}

/*
 *   tests XYZCoord::-=
 */
TEST(XYZCoordOperations, minusEquals) {
  XYZCoord mutated_coord{0, 0, 0};
  EXPECT_EQ(mutated_coord, (XYZCoord{0, 0, 0}));

  mutated_coord -= XYZCoord{0, 0, 0};
  EXPECT_EQ(mutated_coord, (XYZCoord{0, 0, 0}));

  mutated_coord -= XYZCoord{1, 1, 1};
  EXPECT_EQ(mutated_coord, (XYZCoord{-1, -1, -1}));

  mutated_coord -= XYZCoord{15.263, -548526.3, 789.63};
  EXPECT_NEAR(mutated_coord.x, -16.263, 0.001);
  EXPECT_NEAR(mutated_coord.y, 548525.3, 0.001);
  EXPECT_NEAR(mutated_coord.z, -790.63, 0.001);
}

/*
 *   tests XYZCoord::-
 */
TEST(XYZCoordOperations, subtraction) {
  XYZCoord origin{0, 0, 0};
  EXPECT_EQ(origin, (XYZCoord{0, 0, 0}));

  EXPECT_EQ(origin - (XYZCoord{0, 0, 0}), (XYZCoord{0, 0, 0}));

  EXPECT_EQ(origin - (XYZCoord{1, 1, 1}), (XYZCoord{-1, -1, -1}));

  XYZCoord arbitrary_point =
      XYZCoord{-1, -1, -1} - XYZCoord{15.263, -548526.3, 789.63};
  EXPECT_NEAR(arbitrary_point.x, -16.263, 0.001);
  EXPECT_NEAR(arbitrary_point.y, 548525.3, 0.001);
  EXPECT_NEAR(arbitrary_point.z, -790.63, 0.001);
}

/*
 *   tests XYZCoord::*
 *
 *   tests when the scalar is both on the left and right
 *   of the vector
 */
TEST(XYZCoordOperations, scalarMultiplication) {
  XYZCoord ones{1, 1, 1};

  EXPECT_EQ(ones * 1, (XYZCoord{1, 1, 1}));
  EXPECT_EQ(1 * ones, (XYZCoord{1, 1, 1}));
  EXPECT_EQ(1 * ones, ones * 1);

  EXPECT_EQ(ones * 0, (XYZCoord{0, 0, 0}));
  EXPECT_EQ(0 * ones, (XYZCoord{0, 0, 0}));
  EXPECT_EQ(0 * ones, ones * 0);

  EXPECT_EQ(ones * 8, (XYZCoord{8, 8, 8}));
  EXPECT_EQ(8 * ones, (XYZCoord{8, 8, 8}));
  EXPECT_EQ(8 * ones, ones * 8);

  EXPECT_EQ(ones * -2.61, (XYZCoord{-2.61, -2.61, -2.61}));
  EXPECT_EQ(-2.61 * ones, (XYZCoord{-2.61, -2.61, -2.61}));
  EXPECT_EQ(-2.61 * ones, ones * -2.61);

  XYZCoord arbitrary_vector{5.26, 48.54, -4.523};
  XYZCoord multiply_from_left = 3.526 * arbitrary_vector;
  XYZCoord multiply_from_right = arbitrary_vector * 3.526;

  EXPECT_EQ(multiply_from_left, multiply_from_right);

  EXPECT_NEAR(multiply_from_left.x, 18.54676, 0.001);
  EXPECT_NEAR(multiply_from_left.y, 171.15204, 0.001);
  EXPECT_NEAR(multiply_from_left.z, -15.948098, 0.001);

  EXPECT_NEAR(multiply_from_right.x, 18.54676, 0.001);
  EXPECT_NEAR(multiply_from_right.y, 171.15204, 0.001);
  EXPECT_NEAR(multiply_from_right.z, -15.948098, 0.001);
}

/*
 *   tests XYZCoord::norm()
 */
TEST(XYZCoordOperations, norm) {
  XYZCoord origin{0, 0, 0};
  XYZCoord ones{1, 1, 1};
  XYZCoord arbitrary_vector{5.26, 48.54, -4.523};
  XYZCoord norm_less_than_one{0.1, 0.2, 0.3};

  EXPECT_NEAR(origin.norm(), 0, 0.001);
  EXPECT_NEAR(ones.norm(), 1.732, 0.001);
  EXPECT_NEAR(arbitrary_vector.norm(), 49.0332206672, 0.001);
  EXPECT_NEAR(norm_less_than_one.norm(), 0.3741657387, 0.001);
}

/*
 *   tests XYZCoord::normalized()
 */
TEST(XYZCoordOperations, normalized) {
  std::vector<XYZCoord> original_vectors{
      XYZCoord{0, 0, 0},              // origin
      XYZCoord{1, 1, 1},              // ones
      XYZCoord{5.26, 48.54, -4.523},  // arbitrary
      XYZCoord{0.1, 0.2, 0.3}         // norm < one
  };

  std::vector<XYZCoord> expected_normalized{
      XYZCoord{0, 0, 0},                                  // origin
      XYZCoord{0.57736721, 0.57736721, 0.57736721},       // ones
      XYZCoord{0.1072742098, 0.9899410918, -0.09224358},  // arbitrary
      XYZCoord{0.26726124, 0.53452248, 0.80178372}        // norm < one
  };

  for (int i = 0; i < original_vectors.size(); i++) {
    XYZCoord normalized_vector = original_vectors[i].normalized();

    if (normalized_vector.norm() != 0) {
      EXPECT_NEAR(normalized_vector.norm(), 1, 0.001);
    }

    EXPECT_NEAR(normalized_vector.x, expected_normalized[i].x, 0.001);
    EXPECT_NEAR(normalized_vector.y, expected_normalized[i].y, 0.001);
    EXPECT_NEAR(normalized_vector.z, expected_normalized[i].z, 0.001);
  }
}

/*
 *   tests Polygon::pointInBounds
 */
TEST(Polygon, PointInBounds) {
  Polygon test{FLIGHT_BOUND_COLOR};
  test.emplace_back(XYZCoord{1, 1, 0});
  test.emplace_back(XYZCoord{0, 1, 0});
  test.emplace_back(XYZCoord{0, 0, 0});
  test.emplace_back(XYZCoord{1, 0, 0});

  EXPECT_EQ(true, test.pointInBounds(XYZCoord{0.5, 0.5, 0}));
  EXPECT_EQ(true, test.pointInBounds(XYZCoord{0.5, 0.5, 99999999}));

  EXPECT_EQ(false, test.pointInBounds(XYZCoord{1, 0.5, 0}));  // edge is outside
  EXPECT_EQ(false, test.pointInBounds(XYZCoord{2, 0.5, 0}));  // right
  EXPECT_EQ(false, test.pointInBounds(XYZCoord{0.5, 2, 0}));  // top
  EXPECT_EQ(false, test.pointInBounds(XYZCoord{-1, 0.5, 0}));  // left
  EXPECT_EQ(false, test.pointInBounds(XYZCoord{0.5, -1, 0}));  // down

  Polygon point{FLIGHT_BOUND_COLOR};
  test.emplace_back(XYZCoord{1, 1, 1});

  EXPECT_EQ(false, point.pointInBounds(XYZCoord{1, 1, 1}));
  EXPECT_EQ(false, point.pointInBounds(XYZCoord{1, 0, 1}));
  EXPECT_EQ(false, point.pointInBounds(XYZCoord{0, 1, 1}));

  // tests close to diagonals
  Polygon quadrateral{FLIGHT_BOUND_COLOR};
  quadrateral.emplace_back(XYZCoord{0, 0, 0});
  quadrateral.emplace_back(XYZCoord{2, 1, 0});
  quadrateral.emplace_back(XYZCoord{4, 4, 0});
  quadrateral.emplace_back(XYZCoord{1, 2, 0});

  EXPECT_EQ(true, quadrateral.pointInBounds(XYZCoord{1.5, 1.00, 0}));
  EXPECT_EQ(true, quadrateral.pointInBounds(XYZCoord{0.5, 0.90, 0}));
  EXPECT_EQ(true, quadrateral.pointInBounds(XYZCoord{2.5, 2.00, 0}));
  EXPECT_EQ(true, quadrateral.pointInBounds(XYZCoord{1.5, 2.25, 0}));

  EXPECT_EQ(false, quadrateral.pointInBounds(XYZCoord{1.5, 0.75, 0}));
  EXPECT_EQ(false, quadrateral.pointInBounds(XYZCoord{0.5, 1.10, 0}));
  EXPECT_EQ(false, quadrateral.pointInBounds(XYZCoord{2.5, 1.30, 0}));
  EXPECT_EQ(false, quadrateral.pointInBounds(XYZCoord{1.5, 2.50, 0}));
}