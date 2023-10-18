#include "../include/pathing/cartesian.hpp"
#include <gtest/gtest.h>

/*
* Verify haversine formula calculations.
*/
TEST(CartesianTest, havDistanceTest) {
  struct point startPoint;
  startPoint.lat_x = 0;
  startPoint.lng_y = 0;
  startPoint.alt_z = 0;
  CartesianConverter testConverter = CartesianConverter(&startPoint);
  // 99.99% accuracy; Haversine distance of LatLong (0,0) to UCSD Giesel Library LatLong (32.8811, -117.2376)
  // hav(lat1 - lat2) + hav(dLng) * cos(lat1) * cos(lat2)
  // hav(0 - 32.8811) + hav(0 - -117.2376) * cos(0) * cos(32.8811) = 0.52854000101700005
  // EXPECT_EQ(testConverter.havDistance(0, 32.8811, 117.2376), 0.528540001017);
  EXPECT_TRUE(testConverter.havDistance(0, 32.8811, 117.2376) > .9999*0.528540001017);
  EXPECT_TRUE(testConverter.havDistance(0, 32.8811, 117.2376) < 1.0001*0.528540001017);
}
TEST(CartesianTest, distanceRadiansTest) {
  struct point startPoint;
  startPoint.lat_x = 0;
  startPoint.lng_y = 0;
  startPoint.alt_z = 0;
  CartesianConverter testConverter = CartesianConverter(&startPoint);
  // 99.99% accuracy; Unit sphere radians distance between LatLong (0,0) to UCSD Giesel Library LatLong (32.8811, -117.2376)
  // arcHav(havDistance(lat1, lat2, lng1 - lng2))
  // arcHav(0.52854000101700005) = 2 * asin(sqrt(0.52854000101700005)) = 1.62790737001
  // EXPECT_EQ(testConverter.distanceRadians(0, 0, 32.8811, 117.2376), 1.62790737001);
  EXPECT_TRUE(testConverter.distanceRadians(0, 0, 32.8811, 117.2376) > .9999*1.62790737001);
  EXPECT_TRUE(testConverter.distanceRadians(0, 0, 32.8811, 117.2376) < 1.0001*1.62790737001);
}
TEST(CartesianTest, angleRadiansTest) {
  struct point startPoint;
  startPoint.lat_x = 0;
  startPoint.lng_y = 0;
  startPoint.alt_z = 0;
  CartesianConverter testConverter = CartesianConverter(&startPoint);
  // 99.99% accuracy; Unit circle radian angle between LatLong (0,0) to UCSD Giesel Library LatLong (32.8811, -117.2376)
  // distanceRadians(toRadians(std::get<0>(from)), toRadians(std::get<1>(from)), toRadians (std::get<0>(to)), toRadians(std::get<1>(to)))
  // distanceRadians(toRadians(0), toRadians(0), toRadians (32.8811), toRadians(-117.2376))
  // distanceRadians(0, 0, 0.573883456678, -2.04618212714)
  // arcHav(havDistance(0, 0.573883456678, 0 + 2.04618212714))
  // arcHav(hav(0 - 0.573883456678) + hav(2.04618212714) * cos(0) * cos(0.573883456678))
  // arcHav(0.0801005028496 + 0.728840751287 * 1 * 0.839798994301)
  // 2 * asin(sqrt(0.692180232786)) = 1.96531130008
  // EXPECT_EQ(testConverter.computeAngleBetween(std::make_tuple(0,0), std::make_tuple(32.8811, -117.2376)), 1.96531130008);
  EXPECT_TRUE(testConverter.computeAngleBetween(std::make_tuple(0,0), std::make_tuple(32.8811, -117.2376)) > .9999*1.96531130008);
  EXPECT_TRUE(testConverter.computeAngleBetween(std::make_tuple(0,0), std::make_tuple(32.8811, -117.2376)) < 1.0001*1.96531130008);
}
TEST(CartesianTest, distanceBetweenLatLongsTest) {
  struct point startPoint;
  startPoint.lat_x = 0;
  startPoint.lng_y = 0;
  startPoint.alt_z = 0;
  CartesianConverter testConverter = CartesianConverter(&startPoint);
  // 1.96531130008 * EARTH_RADIUS (6371008.7714) = 12521015.5313
  // EXPECT_EQ(testConverter.computeDistanceBetween(std::make_tuple(0,0), std::make_tuple(32.8811, -117.2376)), 12521015.5313);
  EXPECT_TRUE(testConverter.computeDistanceBetween(std::make_tuple(0,0), std::make_tuple(32.8811, -117.2376)) > .9999*12521015.5313);
  EXPECT_TRUE(testConverter.computeDistanceBetween(std::make_tuple(0,0), std::make_tuple(32.8811, -117.2376)) < 1.0001*12521015.5313);
  // This matches up with (0,0) -> (32.8811, -117.2376) LatLongs = 12513000m on the NHC CPHC tool linked above.
}
TEST(CartesianTest, XYtoLatLongTest) {
  struct point startPoint;
  startPoint.lat_x = 0;
  startPoint.lng_y = 0;
  startPoint.alt_z = 0;
  CartesianConverter testConverter = CartesianConverter(&startPoint);

  struct point endPoint;
  endPoint.lat_x = 3654000;
  endPoint.lng_y = 13027000;
  endPoint.alt_z = 0;

  struct point p_result = testConverter.xy_to_latlng(&endPoint);

  // Null Island XY meters (0,0) to Giesel Library, United States XY meters (3654000,13027000) -> (32.8811, -117.2376) in LatLong
  // EXPECT_EQ(testConverter.xy_to_latlng(3654000,13027000), std::make_tuple(32.8811, -117.2376));
  EXPECT_TRUE(p_result.lat_x > .999*32.8811);
  EXPECT_TRUE(p_result.lat_x < 1.001*32.8811);
  // Note: will return only positive coordinates, as angles cannot be uniquely converted back to longitudes and latitudes.
  EXPECT_TRUE(p_result.lng_y > .999*117.2376);
  EXPECT_TRUE(p_result.lng_y < 1.001*117.2376);
}

/*
* Test null point distance calculations.
*/
TEST(CartesianTest, PointZeroZeroEquality) {
  struct point startPoint;
  startPoint.lat_x = 0;
  startPoint.lng_y = 0;
  startPoint.alt_z = 0;
  CartesianConverter testConverter = CartesianConverter(&startPoint);

  struct point endPoint;
  endPoint.lat_x = 0;
  endPoint.lng_y = 0; 
  endPoint.alt_z = 0;
  struct point resultPoint = testConverter.latlng_to_xy(&endPoint);

  // LatLong (0,0,0) to LatLong (0,0,0) -> XYZ (0,0,0)
  EXPECT_EQ(resultPoint.lat_x, 0);
  EXPECT_EQ(resultPoint.lng_y, 0);
  EXPECT_EQ(resultPoint.alt_z, 0);

  // XYZ (0,0,0) to XYZ (0,0,0) -> LatLong (0,0,0)
  EXPECT_EQ(resultPoint.lat_x, 0);
  EXPECT_EQ(resultPoint.lng_y, 0);
  EXPECT_EQ(resultPoint.alt_z, 0);
}

/*
* Double check coordinate conversions to a 99.9% margin of accuracy with
* U.S. National Hurricane Center and Central Pacific Hurricane Center tool.
* NOTE: The tool rounds to closest km, so values are approximate.
* In short distance tests, calculations are unreliable due to tool benchmark rounding.
* https://www.nhc.noaa.gov/gccalc.shtml
*
* Altitude of GPS coordinates are calculated from:
* https://www.dcode.fr/earth-elevation
* Altitude is retained from the starting point.
*/
TEST(CartesianTest, LongDist_LatLongTest) {
  struct point startPoint;
  startPoint.lat_x = 0;
  startPoint.lng_y = 0;
  startPoint.alt_z = 0;
  CartesianConverter testConverter = CartesianConverter(&startPoint);

  // Northwest - 99.9% accuracy
  // Null Island LatLong (0,0,0) to Giesel Library, United States LatLong (32.8811, -117.2376, 112.5300) -> XYZ meters (3654000,13027000,0)
  struct point g_spot;
  g_spot.lat_x = 32.8811;
  g_spot.lng_y = -117.2376;
  g_spot.alt_z = 112.5300;
  struct point g_result = testConverter.latlng_to_xy(&g_spot);
  EXPECT_TRUE(g_result.lat_x > .999*3654*1000); // 3659089433.9999995
  EXPECT_TRUE(g_result.lat_x < 1.001*3654*1000);
  EXPECT_TRUE(g_result.lng_y > .999*13027*1000); // 13036244.27986603
  EXPECT_TRUE(g_result.lng_y < 1.001*13027*1000);
  EXPECT_TRUE(g_result.alt_z == startPoint.alt_z);

  // Northeast - 99.9% accuracy
  // Null Island LatLong (0,0) to CCTV Headquarters, China LatLong (39.9153, 116.4642, 42.0000) -> (4435000,12942000,0) in XY meters
  struct point hq_spot;
  hq_spot.lat_x = 39.9153;
  hq_spot.lng_y = 116.4642;
  hq_spot.alt_z = 42.0000;
  struct point hq_result = testConverter.latlng_to_xy(&hq_spot);
  EXPECT_TRUE(hq_result.lat_x > .999*4435*1000);
  EXPECT_TRUE(hq_result.lat_x < 1.001*4435*1000);
  EXPECT_TRUE(hq_result.lng_y > .999*12942*1000);
  EXPECT_TRUE(hq_result.lng_y < 1.001*12942*1000);
  EXPECT_TRUE(hq_result.alt_z == startPoint.alt_z);

  // Southeast - 99.8% accuracy
  // Null Island LatLong (0,0) to Ironbottom Sound, Solomon Islands (9.2286, -159.9550, 0) -> (1025,17774,0) in XY meters
  struct point i_spot;
  i_spot.lat_x = 9.2286;
  i_spot.lng_y = -159.9550;
  i_spot.alt_z = 0;
  struct point i_result = testConverter.latlng_to_xy(&i_spot);
  EXPECT_TRUE(i_result.lat_x > .999*1025*1000);
  EXPECT_TRUE(i_result.lat_x < 1.002*1025*1000);
  EXPECT_TRUE(i_result.lng_y > .998*17774*1000);
  EXPECT_TRUE(i_result.lng_y < 1.002*17774*1000);
  EXPECT_TRUE(i_result.alt_z == startPoint.alt_z);

  // Southwest - 99.9% accuracy
  // Null Island LatLong (0,0) to Christ the Redeemer, Brazil LatLong (-22.9519, -43.2105, 593.0000) -> (2550000,4802000,0) in XY meters
  struct point c_spot;
  c_spot.lat_x = -22.9519;
  c_spot.lng_y = -43.2105;
  c_spot.alt_z = 593.0000;
  struct point c_result = testConverter.latlng_to_xy(&c_spot);
  EXPECT_TRUE(c_result.lat_x > .999*2550*1000);
  EXPECT_TRUE(c_result.lat_x < 1.001*2550*1000);
  EXPECT_TRUE(c_result.lng_y > .999*4802*1000);
  EXPECT_TRUE(c_result.lng_y < 1.001*4802*1000);
  EXPECT_TRUE(c_result.alt_z == startPoint.alt_z);
}

TEST(CartesianTest, ShortDist_LatLongTest) {
  struct point startPoint;
  startPoint.lat_x = 32.8811;
  startPoint.lng_y = -117.2376;
  startPoint.alt_z = 112.5300;
  CartesianConverter testConverter = CartesianConverter(&startPoint);

  // Southwest - 81.4% accuracy
  // UCSD Giesel Library LatLong (32.8811, -117.2376, 112.5300) to La Jolla Cove, San Diego LatLong (32.8504, -117.2503, 28.0000) -> (3000,1000,112.5300) in XY meters
  struct point j_spot;
  j_spot.lat_x = 32.8504;
  j_spot.lng_y = -117.2503;
  j_spot.alt_z = 28.0000;
  struct point j_result = testConverter.latlng_to_xy(&j_spot);
  EXPECT_TRUE(j_result.lat_x > .814*3*1000);
  EXPECT_TRUE(j_result.lat_x < 1.186*3*1000);
  EXPECT_TRUE(j_result.lng_y > .814*1*1000);
  EXPECT_TRUE(j_result.lng_y < 1.186*1*1000); // 1185.9452
  EXPECT_TRUE(j_result.alt_z == startPoint.alt_z);

  // Southeast - 97.2% accuracy
  // UCSD Giesel Library LatLong (32.8811, -117.2376, 112.5300) to USS Midway, San Diego LatLong (32.7137,-117.1751,7.0000) -> (19000,6000,112.5300) in XY meters
  struct point m_spot;
  m_spot.lat_x = 32.7137;
  m_spot.lng_y = -117.1751;
  m_spot.alt_z = 7.0000;
  struct point m_result = testConverter.latlng_to_xy(&m_spot);
  EXPECT_TRUE(m_result.lat_x > .972*19*1000);
  EXPECT_TRUE(m_result.lat_x < 1.0038*19*1000);
  EXPECT_TRUE(m_result.lng_y > .972*6*1000);
  EXPECT_TRUE(m_result.lng_y < 1.0038*6*1000);
  EXPECT_TRUE(m_result.alt_z == startPoint.alt_z);

  // Northeast - 77.0% accuracy
  // UCSD Giesel Library LatLong (32.8811, -117.2376, 112.5300) to Qualcomm Headquarters LatLong (32.894964, -117.197853, 94.0000) -> (2000,4000,112.5300) in XY meters
  struct point qhq_spot;
  qhq_spot.lat_x = 32.894964;
  qhq_spot.lng_y = -117.197853;
  qhq_spot.alt_z = 94.0000;
  struct point qhq_result = testConverter.latlng_to_xy(&qhq_spot);
  EXPECT_TRUE(qhq_result.lat_x > .77*2*1000);
  EXPECT_TRUE(qhq_result.lat_x < 1.3*2*1000);
  EXPECT_TRUE(qhq_result.lng_y > .77*4*1000);
  EXPECT_TRUE(qhq_result.lng_y < 1.3*4*1000);
  EXPECT_TRUE(qhq_result.alt_z == startPoint.alt_z);

  // Northwest - 99.9% accuracy
  // UCSD Giesel Library LatLong (32.8811, -117.2376, 112.5300) to USS Hornet, Alameda LatLong (37.7726,-122.3025, 3.0000) -> (544000,473000,112.5300) XY meters
  struct point h_spot;
  h_spot.lat_x = 37.7726;
  h_spot.lng_y = -122.3025;
  h_spot.alt_z = 3.0000;
  struct point h_result = testConverter.latlng_to_xy(&h_spot);
  EXPECT_TRUE(h_result.lat_x > .999*544*1000); // 543910.73252066434 
  EXPECT_TRUE(h_result.lat_x < 1.001*544*1000);
  EXPECT_TRUE(h_result.lng_y > .999*473*1000); // 472922.62812608434
  EXPECT_TRUE(h_result.lng_y < 1.001*473*1000);
  EXPECT_TRUE(h_result.alt_z == startPoint.alt_z);
}




