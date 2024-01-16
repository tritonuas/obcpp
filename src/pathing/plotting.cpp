#include <vector>
#include <string>

#include "pathing/plotting.hpp"
#include "utilities/datatypes.hpp"

void PathingPlot::addPoints(std::vector<XYZCoord> pts) {
    for (auto pt : pts) {
        this->addPoint(pt);
    }
}

void PathingPlot::addPolygons(std::vector<Polygon> polygons) {
    for (auto polygon : polygons) {
        this->addPolygon(polygon);
    }
}

void PathingPlot::addPolylines(std::vector<Polyline> polylines) {
    for (auto polyline : polylines) {
        this->addPolyline(polyline);
    }
}


AnimationPathingPlot::AnimationPathingPlot() {
    // TODO: init member vars
}

void AnimationPathingPlot::beginUpdate() {
    // TODO: delineate update
}

void AnimationPathingPlot::endUpdate() {
    // TODO: delinate end of update
}

void AnimationPathingPlot::addPoint(XYZCoord pt) {
    // TODO: add point
}

void AnimationPathingPlot::addPolygon(Polygon polygon) {
    // TODO: add polygon
}

void AnimationPathingPlot::addPolyline(Polyline polyline) {
    // TODO: add polyline
}

void AnimationPathingPlot::output(std::string filename) {
    // TODO: go through all the data and create GIF
    // append .gif to filename param
}


StaticPathingPlot::StaticPathingPlot() {
    // TODO: init member vars
}

void StaticPathingPlot::beginUpdate() {
    // does nothing
}

void StaticPathingPlot::endUpdate() {
    // does nothing
}

void StaticPathingPlot::addPoint(XYZCoord pt) {
    // TODO: add point
}

void StaticPathingPlot::addPolygon(Polygon polygon) {
    // TODO: add polygon
}

void StaticPathingPlot::addPolyline(Polyline polyline) {
    // TODO: add polyline
}

void StaticPathingPlot::output(std::string filename) {
    // TODO: go through all the data and create image
    // append .png to filename param
}
