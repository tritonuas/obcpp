#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <thread>
#include <utility>
#include <filesystem>

#include <assert.h>
#include <Magick++.h>

#include "pathing/plotting.hpp"
#include "utilities/datatypes.hpp"

PathingPlot::PathingPlot() :
    figure(matplot::gcf()) {

    // https://alandefreitas.github.io/matplotplusplus/coding-styles/reactive-vs-quiet-figures/
    this->figure->quiet_mode(true);

    // multiple calls to plot won't erase axes
    this->figure->current_axes()->hold(true);
}

PathingPlot::PathingPlot(Polygon flightBoundary, Polygon aidropBoundary, std::vector<XYZCoord> waypoints) :
    figure(matplot::gcf()),
    flightBoundary(flightBoundary),
    airdropBoundary(aidropBoundary),
    waypoints(waypoints) {

    // https://alandefreitas.github.io/matplotplusplus/coding-styles/reactive-vs-quiet-figures/
    this->figure->quiet_mode(true);

    // multiple calls to plot won't erase axes
    this->figure->current_axes()->hold(true);
}

void PathingPlot::addFinalPoints(std::vector<XYZCoord> pts) {
    for(XYZCoord& coord : pts)  {
        this->addFinalPoint(coord);
    }
}

void PathingPlot::addFinalPolylines(std::vector<Polyline> polylines) {
   for(Polyline& polyline : polylines)  {
        this->addFinalPolyline(polyline);
   }
}

void PathingPlot::addFinalPoint(XYZCoord pt) {
    // if there are no chunks, create an initial empty one
    if (this->plottingChunks.empty()) {
        this->plottingChunks.push_back(PlottingChunk());
    }

    // add a finalItem to the latest chunk
    this->plottingChunks.back().finalItem = pt;
    // create a new chunk for the next set of intermediates and another final
    this->plottingChunks.push_back(PlottingChunk());
}

void PathingPlot::addFinalPolyline(Polyline polyline) {
    // if there are no chunks, create an initial empty one
    if (this->plottingChunks.empty()) {
        this->plottingChunks.push_back(PlottingChunk());
    }

    // add a finalItem to the latest chunk
    this->plottingChunks.back().finalItem = polyline;
    // create a new chunk for the next set of intermediates and another final
    this->plottingChunks.push_back(PlottingChunk());
}

void PathingPlot::addIntermediatePoints(std::vector<XYZCoord> pts) {
    for(XYZCoord& coord : pts)  {
        this->addIntermediatePoint(coord);
    }
}

void PathingPlot::addIntermediatePolylines(std::vector<Polyline> polylines) {
   for(Polyline& polyline : polylines)  {
        this->addIntermediatePolyline(polyline);
   }
}

void PathingPlot::addIntermediatePoint(XYZCoord pt) {
    // if there are no chunks, create an initial empty one
    if (this->plottingChunks.empty()) {
        this->plottingChunks.push_back(PlottingChunk());
    }

    this->plottingChunks.back().intermediateItems.push_back(pt);
}

void PathingPlot::addIntermediatePolyline(Polyline polyline) {
    // if there are no chunks, create an initial empty one
    if (this->plottingChunks.empty()) {
        this->plottingChunks.push_back(PlottingChunk());
    }

    this->plottingChunks.back().intermediateItems.push_back(polyline);
}

void PathingPlot::output(std::string filename, PathOutputType pathType) {
    // TODO: save this to the repo root. right now it just saves to the cwd
    std::filesystem::create_directories("plots");

    std::filesystem::path staticFilepath("plots/" + filename + ".jpg");
    std::filesystem::path animatedFilepath("plots/" + filename + ".gif");

    auto ax = this->figure->current_axes();
    switch (pathType) {
    case PathOutputType::NONE:
        return;
    case PathOutputType::STATIC:
        outputStatic(ax, staticFilepath);
        return;
    case PathOutputType::ANIMATED:
        outputAnimated(ax, animatedFilepath);
        return;
    case PathOutputType::BOTH:
        outputStatic(ax, staticFilepath);
        outputAnimated(ax, animatedFilepath);
        return;
    default:
        return;
    }
}

void PathingPlot::outputStatic(matplot::axes_handle ax, std::filesystem::path filepath) {
    this->clearPlot(ax);
    this->plotStaticBackground(ax);

    // add all final coords
    for (PlottingChunk& chunk: plottingChunks) {
        // skip all chunks without final items
        if (!chunk.finalItem.has_value()) {
            continue;
        }

        this->plotStaticPlottable(ax, chunk.finalItem.value(), PLANNED_PATH_COLOR);
    }

    matplot::save(this->figure, filepath.string());
}


void PathingPlot::outputAnimated(matplot::axes_handle ax, std::filesystem::path filepath) {
    this->currFrame = 0;
    this->clearPlot(ax);
    this->plotStaticBackground(ax);

    // set limits of axes to allocate enough space for the whole graph
    std::pair<double, double> xLimits; 
    std::pair<double, double> yLimits; 
    std::tie(xLimits, yLimits) = this->getAnimatedPlotLimits();
    ax->xlim({xLimits.first, xLimits.second});
    ax->ylim({yLimits.first, yLimits.second});

    // create a temporary directory for the frames
    std::filesystem::path tmpFrameDir = std::filesystem::temp_directory_path() / "frames";
    std::filesystem::create_directories(tmpFrameDir);

    // plot each chunk's intermediate and final items
    for (PlottingChunk& chunk: this->plottingChunks) {
        matplot::axes_type* prevFinalAxRaw = new matplot::axes_type(this->figure);
        std::memcpy(prevFinalAxRaw, ax.get(), sizeof(matplot::axes_type));
        matplot::axes_handle prevFinalAxShared = std::make_shared<matplot::axes_type>(*prevFinalAxRaw);

        // plot intermediates onto temporary axes
        for (Plottable& item: chunk.intermediateItems) {
            this->plotAnimatedPlottable(ax, item, TENTATIVE_PATH_COLOR, tmpFrameDir);
        }

        // plot final item if there is one
        if (chunk.finalItem.has_value()) {
            // plot final item onto temporary Ax
            this->plotAnimatedPlottable(ax, chunk.finalItem.value(), PLANNED_PATH_COLOR, tmpFrameDir);

            // update the current ax to the version before we added any intermediates
            *ax = *prevFinalAxShared;
            // add an extra frame of the finalized item
            this->plotStaticPlottable(ax, chunk.finalItem.value(), PLANNED_PATH_COLOR);
        }
    }

    // use ImageMagick to save the frames to a .gif file
    std::vector<Magick::Image> frames;
    for (int i = 0; i < this->currFrame; i++) {
        Magick::Image image;
        // This isn't great but we assume that the temporary frames at these file paths
        std::filesystem::path frameFilepath = tmpFrameDir / ("frame" + std::to_string(i) + ".jpg");
        image.read(frameFilepath);
        image.animationDelay(this->animationDelayCentiSec);
        frames.push_back(image);
    }
    Magick::writeImages(frames.begin(), frames.end(), filepath);

    // cleanup temporary gifs and temporary directory
    for (int i = 0; i < this->currFrame; i++) {
        std::filesystem::path frameFilepath = tmpFrameDir / ("frame" + std::to_string(i) + ".jpg");
        std::filesystem::remove(frameFilepath);
    }
    std::filesystem::remove(tmpFrameDir);
}

void PathingPlot::clearPlot(matplot::axes_handle ax) {
    ax->clear();
}

void PathingPlot::plotStaticBackground(matplot::axes_handle ax) {
    if (this->flightBoundary.has_value()) {
        this->plotStaticPolygon(ax, flightBoundary.value(), FLIGHT_BOUND_COLOR);
    }

    if (this->airdropBoundary.has_value()) {
        this->plotStaticPolygon(ax, airdropBoundary.value(), AIRDROP_BOUND_COLOR);
    }

    if (this->waypoints.has_value()) {
        for (XYZCoord& coord: this->waypoints.value()) {
            this->plotStaticCoord(ax, coord, WAYPOINTS_COLOR);
        }
    }
}

void PathingPlot::plotStaticPolygon(matplot::axes_handle ax, Polygon& polygon, matplot::color color) {
    for (size_t i = 0; i < polygon.size(); i++) {
        size_t firstPointIdx = i;
        size_t secondPointIdx = i+1;

        // if we reached the end, wrap around and 
        // draw a line from the last point to the first
        if (secondPointIdx == polygon.size()) {
            secondPointIdx = 0;
        }

        XYZCoord& firstPoint = polygon.at(firstPointIdx);
        XYZCoord& secondPoint = polygon.at(secondPointIdx);
        ax->plot({firstPoint.x, secondPoint.x}, {firstPoint.y, secondPoint.y})->line_width(this->lineWidth).color(color);
    }
}

void PathingPlot::plotStaticPolyline(matplot::axes_handle ax, Polyline& polyline, matplot::color color) {
    for (size_t i = 0; i < polyline.size() - 1; i++) {
        size_t firstPointIdx = i;
        size_t secondPointIdx = i+1;

        XYZCoord& firstPoint = polyline.at(firstPointIdx);
        XYZCoord& secondPoint = polyline.at(secondPointIdx);
        ax->plot({firstPoint.x, secondPoint.x}, {firstPoint.y, secondPoint.y})->line_width(this->lineWidth).color(color);
    }
}

void PathingPlot::plotStaticCoord(matplot::axes_handle ax, XYZCoord &coord, matplot::color color) {
    ax->scatter({coord.x}, {coord.y})->
        marker_face(true).
        marker_color(color).
        marker_face_color(color).
        marker_size(this->coordSize).
        color(color);
}

void PathingPlot::plotStaticPlottable(matplot::axes_handle ax, Plottable& plottable, matplot::color color) {
    if(auto *coord = std::get_if<XYZCoord>(&plottable)) {
        this->plotStaticCoord(ax, *coord, color);
    } else if(auto *polyline = std::get_if<Polyline>(&plottable)) {
        this->plotStaticPolyline(ax, *polyline, color);
    } 
}

void PathingPlot::plotAnimatedPolyline(matplot::axes_handle ax, Polyline& polyline, matplot::color color, std::filesystem::path tmpFrameDir) {
    for (size_t currPolyline = 0; currPolyline < polyline.size() - 1; currPolyline++) {
        size_t firstPointIdx = currPolyline;
        size_t secondPointIdx = currPolyline+1;

        XYZCoord& firstPoint = polyline.at(firstPointIdx);
        XYZCoord& secondPoint = polyline.at(secondPointIdx);

        // find out how many frames to generate per line segment
        double segmentDistance = std::sqrt(std::pow(secondPoint.y - firstPoint.y, 2) + std::pow(secondPoint.x - firstPoint.x, 2));
        int numFrames = std::ceil(segmentDistance * this->framesPerDistanceUnit);

        double x_step = (1.f / (double)numFrames)*(secondPoint.x - firstPoint.x);
        double y_step = (1.f / (double)numFrames)*(secondPoint.y - firstPoint.y);

        double curr_x = firstPoint.x;
        double curr_y = firstPoint.y;
        double next_x = curr_x + x_step;
        double next_y = curr_y + y_step;

        for (int i = 0; i < numFrames; i++) {
            ax->plot({curr_x, next_x}, {curr_y, next_y})->line_width(4).color(color);

            // TODO: don't save to build dir
            std::filesystem::path frameFilepath = tmpFrameDir / ("frame" + std::to_string(this->currFrame) + ".jpg");
            ++this->currFrame;

            // TODO: this is so dumb; can't figure out how to get matplotplus plot
            // rendered to a c++ array of pixel vals
            // So, we must save it to disk and load it again via imagemagick for GIF
            matplot::save(this->figure, frameFilepath.string());

            // step forward
            curr_x = next_x;
            next_x += x_step;

            curr_y = next_y;
            next_y += y_step;
        }
    }
}

void PathingPlot::plotAnimatedPlottable(matplot::axes_handle ax, Plottable& plottable, matplot::color color, std::filesystem::path tmpFrameDir) {
    if(auto *coord = std::get_if<XYZCoord>(&plottable)) {
        this->plotStaticCoord(ax, *coord, color);
    } else if(auto *polyline = std::get_if<Polyline>(&plottable)) {
        this->plotAnimatedPolyline(ax, *polyline, color, tmpFrameDir);
    } 
}

std::pair<std::pair<double, double>, std::pair<double, double>> PathingPlot::getAnimatedPlotLimits() {
    double minX = std::numeric_limits<double>::infinity();
    double minY = std::numeric_limits<double>::infinity();
    double maxX = -std::numeric_limits<double>::infinity();
    double maxY = -std::numeric_limits<double>::infinity();

    auto updateLimits = [&minX, &minY, &maxX, &maxY](XYZCoord& coord) mutable {
            if (coord.x < minX) {
               minX = coord.x; 
            }
            if (coord.x > maxX) {
               maxX = coord.x; 
            }
            if (coord.y < minY) {
               minY = coord.y; 
            }
            if (coord.y > maxY) {
               maxY = coord.y; 
            }
    };

    // check background points
    if (this->flightBoundary.has_value()) {
        for(XYZCoord& coord: this->flightBoundary.value()) {
            updateLimits(coord);
        }
    }

    if (this->airdropBoundary.has_value()) {
        for(XYZCoord& coord: this->airdropBoundary.value()) {
            updateLimits(coord);
        }
    }

    if (this->waypoints.has_value()) {
        for (XYZCoord& coord: this->waypoints.value()) {
            updateLimits(coord);
        }
    }

    // loop through each chunk
    for (PlottingChunk& chunk: this->plottingChunks) {

        // loop through intermediates
        for (Plottable& item: chunk.intermediateItems) {
            if(auto *coord = std::get_if<XYZCoord>(&item)) {
                updateLimits(*coord);
            } else if(auto *polyline = std::get_if<Polyline>(&item)) {
                for (XYZCoord& coord: *polyline) {
                    updateLimits(coord);
                }
            } 
        }

        // check final item of each chunk
        if (!chunk.finalItem.has_value()) {
            continue;
        }
        if(auto *coord = std::get_if<XYZCoord>(&chunk.finalItem.value())) {
            updateLimits(*coord);
        } else if(auto *polyline = std::get_if<Polyline>(&chunk.finalItem.value())) {
            for (XYZCoord& coord: *polyline) {
                updateLimits(coord);
            }
        } 
    }

    return std::make_pair(std::make_pair(minX, maxX), std::make_pair(minY, maxY));
}