#ifndef TESTS_UNIT_PATHING_FIELD_HPP_
#define TESTS_UNIT_PATHING_FIELD_HPP_

#include <cmath>
#include <cstddef>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"

// the state rand_r() is walked from, so a test that samples can be repeated
extern unsigned int seed1;

inline void seedRandom(unsigned int seed) { seed1 = seed; }

// 1000 x 1000 field, nothing in it. A 30m turning radius leaves plenty of room
inline const Polygon FIELD = {{XYZCoord(0, 0, 0), XYZCoord(1000, 0, 0), XYZCoord(1000, 1000, 0),
                               XYZCoord(0, 1000, 0)}};

// a wall that splits the field at x in [480, 520], with a 300m gap at the top
inline const Polygon WALL = {{XYZCoord(480, 0, 0), XYZCoord(520, 0, 0), XYZCoord(520, 700, 0),
                              XYZCoord(480, 700, 0)}};

inline void initOpenField() {
    Environment::init(FIELD, {}, {}, {});
    Dubins::_radius = 30;
    Dubins::_point_separation = 10;
}

inline void initFieldWithWall() {
    Environment::init(FIELD, {}, {}, {WALL});
    Dubins::_radius = 30;
    Dubins::_point_separation = 10;
}

inline bool pathIsInBounds(const std::vector<XYZCoord>& path) {
    for (const XYZCoord& point : path) {
        if (!Environment::isPointInBounds(point)) {
            return false;
        }
    }
    return true;
}

// the index of the first point of the path that lands on a waypoint, searching
// from `from` so that waypoints can be checked in the order they are flown
inline std::size_t indexOfPoint(const std::vector<XYZCoord>& path, const XYZCoord& target,
                                std::size_t from = 0) {
    for (std::size_t i = from; i < path.size(); i++) {
        if (std::hypot(path[i].x - target.x, path[i].y - target.y) < 1e-6) {
            return i;
        }
    }
    return path.size();
}

// the heading the path is flying as it lands on the point at `index`
inline double headingAt(const std::vector<XYZCoord>& path, std::size_t index) {
    const XYZCoord& previous = path[index - 1];
    return std::atan2(path[index].y - previous.y, path[index].x - previous.x);
}

// how far apart two headings are, the short way around
inline double angleBetween(double a, double b) { return std::abs(std::remainder(a - b, TWO_PI)); }

#endif  // TESTS_UNIT_PATHING_FIELD_HPP_
