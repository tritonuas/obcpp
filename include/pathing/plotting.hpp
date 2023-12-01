#ifndef PATHING_PLOTTING_HPP
#define PATHING_PLOTTING_HPP

#include <list>

#include <matplot/matplot.h>
#include "utilities/datatypes.hpp"

class PathingPlot {
    public:
virtual ~PathingPlot() = default;
        void addPoints(std::vector<XYZCoord> pts);
        void addPolygons(std::vector<Polygon> polygons);
        void addPolylines(std::vector<Polyline> polylines);

        // Used to group changes to the plot into logical chunks
        virtual void beginUpdate() = 0; // Call before making changes to the plot
        virtual void endUpdate() = 0; // Call when done making changes to the plot

        virtual void addPolygon(Polygon polygon) = 0;
        virtual void addPolyline(Polyline polyline) = 0;
        virtual void addPoint(XYZCoord pt) = 0;

        // Output to filename. Don't include extension as the subclass 
        // will add the correct extension itself
        virtual void output(std::string filename) = 0;
};

// Saves a record of all of the updates that have been done
// When plotted, creates a GIF
class AnimationPathingPlot : public PathingPlot {
    public:
virtual ~AnimationPathingPlot() = default;
        AnimationPathingPlot();

        void beginUpdate() override;
        void endUpdate() override;

        void addPolygon(Polygon polygon) override;
        void addPolyline(Polyline polyline) override;
        void addPoint(XYZCoord pt) override;

        void output(std::string filename) override;
    private:
        std::list<std::list<XYZCoord>> coords{};
        std::list<std::list<Polygon>> polygons{};
        std::list<std::list<Polyline>> polylines{};
};

// Only records the most recent state of the plot
// When plotted, creates a PNG
class StaticPathingPlot : public PathingPlot {
    public:
virtual ~StaticPathingPlot() = default;
        StaticPathingPlot();

        void beginUpdate() override;
        void endUpdate() override;

        void addPolygon(Polygon polygon) override;
        void addPolyline(Polyline polyline) override;
        void addPoint(XYZCoord pt) override;

        void output(std::string filename) override;
    private:
        std::list<XYZCoord> coords{};
        std::list<Polygon> polygons{};
        std::list<Polyline> polylines{};
};

#endif