#include"../include/pathing/cartesian.hpp"
#include <gtest/gtest.h>

TEST(CartesianTest, PointZeroZeroEquality) {
  std::tuple<double,double> testCenter = {0, 0};
  CartesianConverter testConverter = CartesianConverter(testCenter);

  // Make sure the 0,0 point always gets converted to 0,0
  EXPECT_EQ(testConverter.latlng_to_xy(0,0), std::make_tuple(0,0));
    
}

TEST(CartesianTest, LongDistanceLatLongTest) {
  std::tuple<double,double> testCenter = {0,0};
  CartesianConverter testConverter = CartesianConverter(testCenter);

  // Double check coordinate conversions to a 99.9% margin of accuracy with
  // U.S. National Hurricane Center and Central Pacific Hurricane Center tool.
  // NOTE: The tool rounds to closest km, so values are approximate.
  // In short distance tests, calculations are unreliable due to tool benchmark rounding.
  // https://www.nhc.noaa.gov/gccalc.shtml

  // Null Island LatLong (0,0) to Null Island LatLong (0,0)
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(0,0)) == 0);
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(0,0)) == 0);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,0)) == 0);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,0)) == 0);

  // Northwest - 99.9% accuracy
  // Null Island LatLong (0,0) to Giesel Library, United States LatLong (32.8811, -117.2376) -> (3654000,13027000) in XY meters
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(32.8811,0)) > .999*3654*1000); // 3659089433.9999995
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(32.8811,0)) < 1.001*3654*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,-117.2376)) > .999*13027*1000); // 13036244.27986603
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,-117.2376)) < 1.001*13027*1000);

  // Northeast - 99.9% accuracy
  // Null Island LatLong (0,0) to CCTV Headquarters, China LatLong (39.9153, 116.4642) -> (4435000,12942000) in XY meters
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(39.9153,0)) > .999*4435*1000);
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(39.9153,0)) < 1.001*4435*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,116.4642)) > .999*12942*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,116.4642)) < 1.001*12942*1000);

  // Southeast - 99.8% accuracy
  // Null Island LatLong (0,0) to Ironbottom Sound, Solomon Islands (9.2286, -159.9550) -> (1025,17774) in XY meters
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(9.2286,0)) > .999*1025*1000);
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(9.2286,0)) < 1.002*1025*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,-159.9550)) > .998*17774*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,-159.9550)) < 1.002*17774*1000);

  // Southwest - 99.9% accuracy
  // Null Island LatLong (0,0) to Christ the Redeemer, Brazil LatLong (-22.9519, -43.2105) -> (2550000,4802000) in XY meters
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(-22.9519,0)) > .999*2550*1000);
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(-22.9519,0)) < 1.001*2550*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,-43.2105)) > .999*4802*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,-43.2105)) < 1.001*4802*1000);
}

TEST(CartesianTest, ShortDistanceTest) {
  std::tuple<double,double> testCenter = {32.8811, -117.2376};
  CartesianConverter testConverter = CartesianConverter(testCenter);

  // Southwest - 85.9% accuracy
  // UCSD Giesel Library LatLong (32.881, -117.2376) to La Jolla Cove, San Diego LatLong (32.8504, -117.2503) -> (3000,3000) in XY meters
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(32.8504,0)) > .859*3*1000);
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(32.8504,0)) < 1.141*3*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,-117.2729)) > .859*3*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(0,-117.2729)) < 1.141*3*1000);

  // Southeast - 97.2% accuracy
  // UCSD Giesel Library LatLong (32.8811, -117.2376) to USS Midway, San Diego LatLong (32.7137,-117.1751) -> (19000,6000) in XY meters
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(32.7137,-117.1751)) > .972*19*1000);
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(32.7137,-117.1751)) < 1.0038*19*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(32.7137,-117.1751)) > .972*6*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(32.7137,-117.1751)) < 1.0038*6*1000);

  // Northeast - 77.0% accuracy
  // UCSD Giesel Library LatLong (32.8811, -117.2376) to Qualcomm Headquarters LatLong (32.894964, -117.197853) -> (2000,4000) in XY meters
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(32.894964, -117.197853)) > .77*2*1000);
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(32.894964, -117.197853)) < 1.3*2*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(32.894964, -117.197853)) > .77*4*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(32.894964, -117.197853)) < 1.3*4*1000);

  // Northwest - 99.9% accuracy
  // UCSD Giesel Library LatLong (32.8811, -117.2376) to USS Hornet, Alameda LatLong (37.7726,-122.3025) -> (544000,473000) XY meters
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(37.7726,-122.3025)) > .999*544*1000); // 543910.73252066434 
  EXPECT_TRUE(std::get<0>(testConverter.latlng_to_xy(37.7726,-122.3025)) < 1.001*544*1000);
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(37.7726,-122.3025)) > .999*473*1000); // 472922.62812608434
  EXPECT_TRUE(std::get<1>(testConverter.latlng_to_xy(37.7726,-122.3025)) < 1.001*473*1000);
}

TEST(CartesianTest, havDistanceTest) {
  std::tuple<double,double> testCenter = {0, 0};
  CartesianConverter testConverter = CartesianConverter(testCenter);
  // 99.99% accuracy; Haversine distance of LatLong (0,0) to UCSD Giesel Library LatLong (32.8811, -117.2376)
  // hav(lat1 - lat2) + hav(dLng) * cos(lat1) * cos(lat2)
  // hav(0 - 32.8811) + hav(0 - -117.2376) * cos(0) * cos(32.8811) = 0.52854000101700005
  // EXPECT_EQ(testConverter.havDistance(0, 32.8811, 117.2376), 0.528540001017);
  EXPECT_TRUE(testConverter.havDistance(0, 32.8811, 117.2376) > .9999*0.528540001017);
  EXPECT_TRUE(testConverter.havDistance(0, 32.8811, 117.2376) < 1.0001*0.528540001017);
}

TEST(CartesianTest, distanceRadiansTest) {
  std::tuple<double,double> testCenter = {0, 0};
  CartesianConverter testConverter = CartesianConverter(testCenter);
  // 99.99% accuracy; Unit sphere radians distance between LatLong (0,0) to UCSD Giesel Library LatLong (32.8811, -117.2376)
  // arcHav(havDistance(lat1, lat2, lng1 - lng2))
  // arcHav(0.52854000101700005) = 2 * asin(sqrt(0.52854000101700005)) = 1.62790737001
  // EXPECT_EQ(testConverter.distanceRadians(0, 0, 32.8811, 117.2376), 1.62790737001);
  EXPECT_TRUE(testConverter.distanceRadians(0, 0, 32.8811, 117.2376) > .9999*1.62790737001);
  EXPECT_TRUE(testConverter.distanceRadians(0, 0, 32.8811, 117.2376) < 1.0001*1.62790737001);
}

TEST(CartesianTest, angleRadiansTest) {
  std::tuple<double,double> testCenter = {0, 0};
  CartesianConverter testConverter = CartesianConverter(testCenter);
  // 99.99% accuracy; Unit circle radian angle between LatLong (0,0) to UCSD Giesel Library LatLong (32.8811, -117.2376)
  // distanceRadians(toRadians(std::get<0>(from)), toRadians(std::get<1>(from)), toRadians (std::get<0>(to)), toRadians(std::get<1>(to)))
  // distanceRadians(toRadians(0), toRadians(0), toRadians (32.8811), toRadians(-117.2376))
  // distanceRadians(0, 0, 0.573883456678, -2.04618212714)
  // arcHav(havDistance(0, 0.573883456678, 0 + 2.04618212714))
  // arcHav(hav(0 - 0.573883456678) + hav(2.04618212714) * cos(0) * cos(0.573883456678))
  // arcHav(0.0801005028496 + 0.728840751287 * 1 * 0.839798994301)
  // 2 * asin(sqrt(0.692180232786)) = 1.96531130008
  // EXPECT_EQ(testConverter.computeAngleBetween(testCenter, std::make_tuple(32.8811, -117.2376)), 1.96531130008);
  EXPECT_TRUE(testConverter.computeAngleBetween(testCenter, std::make_tuple(32.8811, -117.2376)) > .9999*1.96531130008);
  EXPECT_TRUE(testConverter.computeAngleBetween(testCenter, std::make_tuple(32.8811, -117.2376)) < 1.0001*1.96531130008);
}

TEST(CartesianTest, distanceBetweenLatLongsTest) {
  std::tuple<double,double> testCenter = {0, 0};
  CartesianConverter testConverter = CartesianConverter(testCenter);
  // 1.96531130008 * EARTH_RADIUS (6371008.7714) = 12521015.5313
  // EXPECT_EQ(testConverter.computeDistanceBetween(testCenter, std::make_tuple(32.8811, -117.2376)), 12521015.5313);
  EXPECT_TRUE(testConverter.computeDistanceBetween(testCenter, std::make_tuple(32.8811, -117.2376)) > .9999*12521015.5313);
  EXPECT_TRUE(testConverter.computeDistanceBetween(testCenter, std::make_tuple(32.8811, -117.2376)) < 1.0001*12521015.5313);

  // This matches up with (0,0) -> (32.8811, -117.2376) LatLongs = 12513000m on the NHC CPHC tool linked above.
}

/*
TEST(CartesianTest, rawFormulaTest) {
  std::tuple<double,double> testCenter = {0, 0};
  CartesianConverter testConverter = CartesianConverter(testCenter);

  // Check raw latlong formula has the same output as with helper functions
  EXPECT_EQ(testConverter.latlng_to_xy(32.8811, -117.2376), testConverter.latlng_to_xy_raw(32.8811, -117.2376));
}
*/

TEST(CartesianTest, XYtoLatLongTest) {
  std::tuple<double,double> testCenter = {0, 0};
  CartesianConverter testConverter = CartesianConverter(testCenter);

  // Null Island XY meters (0,0) to Giesel Library, United States XY meters (3654000,13027000) -> (32.8811, -117.2376) in LatLong
  // EXPECT_EQ(testConverter.xy_to_latlng(3654000,13027000), std::make_tuple(32.8811, -117.2376));
  EXPECT_TRUE(std::get<0>(testConverter.xy_to_latlng(3654000,13027000)) > .999*32.8811);
  EXPECT_TRUE(std::get<0>(testConverter.xy_to_latlng(3654000,13027000)) < 1.001*32.8811);
  // Note: will return only positive coordinates, as angles cannot be uniquely converted back to longitudes and latitudes.
  EXPECT_TRUE(std::get<1>(testConverter.xy_to_latlng(3654000,13027000)) > .999*117.2376);
  EXPECT_TRUE(std::get<1>(testConverter.xy_to_latlng(3654000,13027000)) < 1.001*117.2376);
}






