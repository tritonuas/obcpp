#include "cv/hdr.hpp"
#include <gtest/gtest.h>

/**
 * Ensure HDR computation does not fail.
*/
TEST(HdrTest, compute_hdr_test) {
  // Get some fake image data
  std::vector<cv::Mat> images = generate_test_images();
  cv::Mat exposure_times = get_image_exposure_times();

  int hdr_error_code = compute_hdr(images, get_image_exposure_times());
  EXPECT_EQ(0, hdr_error_code);
}
