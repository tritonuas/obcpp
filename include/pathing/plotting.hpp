#ifndef INCLUDE_PATHING_PLOTTING_HPP_
#define INCLUDE_PATHING_PLOTTING_HPP_

#include <matplot/matplot.h>

#include <list>
#include <string>
#include <vector>
#include <cmath>
#include <variant>
#include <filesystem>
#include <utility>

#include <opencv2/opencv.hpp>

#include "utilities/datatypes.hpp"

// type-safe union to contain types we would want to plot in a path
// (polygons are not included since they are usually only
//    needed during initialization for boundaries)
using Plottable = std::variant<XYZCoord, Polyline>;

// PathType describes what type of path to output
enum class PathOutputType {
    NONE,  // we want this during competition, for performance
    STATIC,  // output to .jpg
    ANIMATED,  // output to .gif
    BOTH  // both STATIC and ANIMATED
};

// PlottingChunk
struct PlottingChunk {
    // intermediateItems contain anything to be plotted
    // that shouldn't be a part of the final result
    std::vector<Plottable> intermediateItems;
    // finalItem contains a persistent item that will be a part of the final result
    std::optional<Plottable> finalItem;
};

class PathingPlot {
 public:
    explicit PathingPlot(std::filesystem::path outputDir);
    // Constructor for initializing with competition boundaries and waypoints.
    // These items will remain in the background of all paths.
    PathingPlot(std::filesystem::path outputDir,
      Polygon flightBoundary, Polygon airdropBoundary, std::vector<XYZCoord> waypoints);

    // Add a point which will be wiped from the graph when the next segment is finalized.
    // Only use for intermediate/temporary items that we are exploring but have not
    // committed to. Note that this intermediate point will only show up when outputing as
    // an animated GIF.
    void addIntermediatePoint(XYZCoord pt);
    // Add a polyline which will be wiped from the graph when the next segment is finalized.
    // Only use for intermediate/temporary items that we are exploring but have not
    // committed to. Note that this intermediate polyline will only show up when outputing as
    // an animated GIF.
    void addIntermediatePolyline(Polyline polyline);

    // Add multiple points which will be wiped from the graph when the next segment is finalized.
    // Only use for intermediate/temporary items that we are exploring but have not
    // committed to. Note that these intermediate points will only show up when outputing as
    // an animated GIF.
    void addIntermediatePoints(std::vector<XYZCoord> pts);
    // Add multiple polylines which will be wiped from the graph when the next segment is finalized.
    // Only use for intermediate/temporary items that we are exploring but have not
    // committed to. Note that these intermediate points will only show up when outputing as
    // an animated GIF.
    void addIntermediatePolylines(std::vector<Polyline> polylines);

    // Add a single point to the final version of the path
    void addFinalPoint(XYZCoord pt);
    // Add a single polyline to the final version of the path
    void addFinalPolyline(Polyline polyline);

    // Add multiple points to the final version of the path
    void addFinalPoints(std::vector<XYZCoord> pts);
    // Add multiple polylines to the final version of the path
    void addFinalPolylines(std::vector<Polyline> polylines);

    // Output to filename. Don't include extension as the function
    // will add the correct extension itself
    void output(std::string filename, PathOutputType pathType);

    // how thick polylines and polygon lines should be
    void setLineWidth(float lineWidth);
    // how large points should be
    void setCoordSize(float coordSize);
    // how much to pause between each frame in 1/100th of a second
    void setAnimationDelay(size_t animationDelayCentiSec);
    // how many frames to generate per unit of distance, how fine/granular the GIF should be
    // ex: if the line moves 1 ft, generate x number of frames to animate that 1ft change
    void setFramesPerDistanceUnit(double framesPerDistanceUnit);

 private:
    std::filesystem::path outputDir;

    std::optional<Polygon> flightBoundary;
    std::optional<Polygon> airdropBoundary;
    std::optional<std::vector<XYZCoord>> waypoints;

    std::list<PlottingChunk> plottingChunks{};

    // only output final points to image
    void outputStatic(matplot::axes_handle ax, std::filesystem::path filepath);

    // output intermediate and final points to gif
    void outputAnimated(matplot::axes_handle ax, std::filesystem::path filepath);

    // top-level matplotplus figure where everything goes
    matplot::figure_handle figure;

    void clearPlot(matplot::axes_handle ax);

    // the following methods modify the axes_handle passed in
    void plotStaticBackground(matplot::axes_handle ax);
    void plotStaticPlottable(matplot::axes_handle ax, const Plottable& plottable,
      matplot::color color);
    void plotStaticPolyline(matplot::axes_handle ax, const Polyline &polyline,
      matplot::color color);
    void plotStaticCoord(matplot::axes_handle ax, const XYZCoord &coord,
      matplot::color color);
    void plotStaticPolygon(matplot::axes_handle ax, const Polygon &polygon,
      matplot::color color);

    // the following two methods modify the axes_handle passed in and
    // save multiple frames to .jpg files
    void plotAnimatedPlottable(matplot::axes_handle ax, const Plottable& plottable,
      matplot::color color, std::filesystem::path tmpFrameDir);
    void plotAnimatedPolyline(matplot::axes_handle ax, const Polyline& polyline,
      matplot::color color, std::filesystem::path tmpFrameDir);

    // internal counter to keep track of which frame we're on
    int currFrame = 0;

    float lineWidth = 4.0f;
    float coordSize = 10.0f;

    // Time in 1/100ths of a second which must expire before
    // displaying the next image in an animated sequence.
    size_t animationDelayCentiSec = 10;

    // how many frames to generate per each unit of distance in a path
    double framesPerDistanceUnit = 0.1f;

    // helper function to find the smallest and largest coordinates
    // from both intermediate and final objects.
    // Returns ((minX, maxX), (minY, maxY))
    std::pair<std::pair<double, double>, std::pair<double, double>> getAnimatedPlotLimits();
};

#endif  // INCLUDE_PATHING_PLOTTING_HPP_
