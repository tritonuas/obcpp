#include <gtest/gtest.h>
#include "cv/hdr.hpp"

TEST(HdrTest,compute_hdr_test) {
  int hdr_error_code = compute_hdr();
  EXPECT_EQ(0, hdr_error_code);
}
