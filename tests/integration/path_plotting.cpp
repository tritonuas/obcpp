#include "pathing/plotting.hpp"
// #include "utilities/datatypes.hpp"
// #include <matplot/matplot.h>
// #include <cmath>

int main() {
    Polygon flightBounds{{XYZCoord(0, 0, 0), XYZCoord(100, 0, 0), XYZCoord(100, 30, 0), XYZCoord(0, 30, 0)}};
    Polygon airdropBounds{{XYZCoord(10, 10, 0), XYZCoord(15, 10, 0), XYZCoord(15, 15, 0), XYZCoord(12.5f, 17.5f, 0), XYZCoord(10, 15, 0)}};
    std::vector<XYZCoord> waypoints = { XYZCoord(10, 40, 0), XYZCoord(45, 16, 0), XYZCoord(22, 30, 0) };

    // can also provide absolute path for plotting output directory
    PathingPlot plotter("plots", flightBounds, airdropBounds, waypoints);

    Polyline path{{XYZCoord(30, 15, 90), XYZCoord(20, 5, 85), XYZCoord(4, 10, 70), XYZCoord(10, 22, 100), XYZCoord(41, 40, 110), XYZCoord(20, 55, 105)}};
    plotter.addIntermediatePolyline(path);

    Polyline finalPath{{XYZCoord(30, 15, 90), XYZCoord(32, 17, 85), XYZCoord(35, 20, 70), XYZCoord(52, 30, 100), XYZCoord(41, 40, 110), XYZCoord(20, 55, 105)}};
    plotter.addFinalPolyline(finalPath);

    Polyline tempPath2{{XYZCoord(20, 55, 105), XYZCoord(60, 55, 105)}};
    plotter.addIntermediatePolyline(tempPath2);

    Polyline finalPath2{{XYZCoord(20, 55, 105), XYZCoord(20, 70, 105)}};
    plotter.addFinalPolyline(finalPath2);

    plotter.output("test_static", PathOutputType::STATIC);
    plotter.output("test_animated", PathOutputType::ANIMATED);
}