#include "pathing/rrt.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "pathing/tree.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/logging.hpp"
#include "utilities/rng.hpp"

namespace {

/*
 * The plane is already sitting on the first goal at the heading it is flying, so
 * that is the only angle it could be said to have reached it at, whatever the
 * caller filled in.
 */
std::vector<std::vector<double>> withStartAngle(std::vector<std::vector<double>> goal_angles,
                                                double start_angle) {
	if (!goal_angles.empty()) {
		goal_angles[0] = {start_angle};
	}

	return goal_angles;
}

}  // namespace

RRT::RRT(std::vector<XYZCoord> goals, double start_angle,
         std::vector<std::vector<double>> goal_angles)
	: tree(RRTPoint(goals[0], start_angle)),
	  goals(std::move(goals)),
	  goal_angles(withStartAngle(std::move(goal_angles), start_angle)) {}

RRT::RRT(std::vector<XYZCoord> goals, double start_angle, std::vector<double> angles)
	: RRT(goals, start_angle,
	      std::vector<std::vector<double>>(goals.size(),
	                                       angles.empty() ? DEFAULT_GOAL_ANGLES : angles)) {}

void RRT::run() {
	generateDubinsOptions();
	generateFlightPoints();
}

void RRT::generateDubinsOptions() {
	/*
	 * RRT algorithm
	 * - Treats each waypoint as a goal, DOES NOT reuse trees between waypoints,
	 *    basically calls RRT for each waypoint
	 * - For Each Waypoint
	 *  - Tries to connect directly to the goal
	 *  - If it can't, it runs the RRT algorithm
	 *      - If it can't, it connects to the goal with whatever it has
	 */
	const uint8_t total_goals = goals.size();

	// the plane is already sitting on the first goal, it is flown from, not to
	for (uint8_t cur_goal_idx = 1; cur_goal_idx < total_goals; cur_goal_idx++) {
		// tries to connect directly to the goal
		if (connectToGoal(cur_goal_idx)) {
			continue;
		}

		// run the RRT algorithm if it can not connect
		RRTIteration(cur_goal_idx);
	}
}

double RRT::pathLength() const {
	double length = 0;

	for (const Leg& leg : legs) {
		length += leg.length;
	}

	return length;
}

void RRT::generateFlightPoints() {
	flight_path.clear();

	for (const Leg& leg : legs) {
		const std::vector<XYZCoord> points = buildFlightPath(leg);
		flight_path.insert(flight_path.end(), points.begin(), points.end());
	}
}

std::vector<XYZCoord> RRT::getPointsToGoal() const { return flight_path; }

bool RRT::RRTIteration(uint8_t cur_goal_idx) {
	// the sample is the only point a given iteration paths to, so the list it is
	// searched with is built once and written over
	std::vector<RRTPoint> sample(1);

	for (NodeId _ = 0; _ < ITERATIONS_PER_WAYPOINT; _++) {
		sample[0] = RRTPoint(Environment::getRandomPoint(false, goals[cur_goal_idx]),
                             random(0, TWO_PI));

		// adds the sample to the tree if there is any way to fly to it
		const Connection connection = bestConnection(sample, MAX_DUBINS_OPTIONS_TO_PARSE);

		if (connection.isValid()) {
	        tree.addSample(connection.anchor, connection.end, connection.option);
		}
	}

	if (connectToGoal(cur_goal_idx)) {
		return true;
	}

	loguru::set_thread_name("Static Pathing");
	LOG_F(
		WARNING,
		"Failed to connect to goal on iteration: [%s]. Trying again...",
		std::to_string(cur_goal_idx).c_str()
	);

	// throws away the tree that failed, keeping the same starting point
	tree.setCurrentHead(tree.getStart());

	// TODO: possiblility for infinite loop
	return RRTIteration(cur_goal_idx);
}

double RRT::lowerBound(NodeId node, const std::vector<RRTPoint>& ends) const {
	const XYZCoord& anchor = tree.tree.points[node].coord;

	double closest = std::numeric_limits<double>::infinity();

	for (const RRTPoint& end : ends) {
		closest = std::min(closest, anchor.distanceTo(end.coord));
	}

	return tree.tree.length[node] + closest;
}

void RRT::fillOptions(NodeId node, const std::vector<RRTPoint>& ends) const {
 	options.clear();
	const RRTPoint& anchor = tree.tree.points[node];
	const double flown = tree.tree.length[node];

	for (const RRTPoint& end : ends) {
		// gets all dubins curves from the given node to the end point
		for (const RRTOption& option : Dubins::allOptions(anchor, end)) {
			// filters out the options that are not valid
			if (!std::isfinite(option.length)) {
				continue;
			}

			options.push_back({node, end, option, flown + option.length});
		}
	}
}

Connection RRT::bestConnection(const std::vector<RRTPoint>& ends, int max_paths_checked) const {
	/* bounds holds the cheapest flight each node could possibly offer, and frontier
	 * the order the nodes are looked at in -- indexed by node and by rank
	 * respectively. The order is kept as a heap because the search normally only ever
	 * reaches the first few nodes, so sorting the whole tree to path from three of it
	 * would be most of the work thrown away.
	 */
	for (NodeId node = 0; node < tree.tree.size; node++) {
		bounds[node] = lowerBound(node, ends);
		frontier[node] = node;
	}

	NodeId remaining = tree.tree.size;
	const auto cheapest_last = [this](NodeId a, NodeId b) { return bounds[a] > bounds[b]; };
	std::make_heap(frontier.begin(), frontier.begin() + remaining, cheapest_last);

	Connection best;
	int paths_checked = 0;

	while (remaining > 0) {
		std::pop_heap(frontier.begin(), frontier.begin() + remaining, cheapest_last);
		const NodeId node = frontier[--remaining];

		// the rest of the tree is at least this expensive, so it cannot do better
		if (bounds[node] >= best.cost) {
			break;
		}

		fillOptions(node, ends);
		std::sort(options.begin(), options.end(),
				  [](const Connection& a, const Connection& b) { return a.cost < b.cost; });

		for (const Connection& option : options) {
			if (option.cost >= best.cost) {
				break;
			}

			if (paths_checked++ >= max_paths_checked) {
				return best;
			}

			/* The options are cheapest first, so the first one that stays inside the
			 * airspace is the best this node has to offer. Only the path is checked --
			 * the node it is flown from was already checked on its way into the tree.
			 *
			 * CSC paths are everything Dubins::allOptions currently generates, and they
			 * are the only ones this can be asked about.
			 */
			if (Environment::isDubinsPathInBounds(tree.tree.points[node], option.end,
												  option.option)) {
				best = option;
				break;
			}
		}
	}

	return best;
}

std::vector<RRTPoint> RRT::goalEndpoints(int cur_goal_idx) const {
	std::vector<RRTPoint> ends;
	ends.reserve(goal_angles[cur_goal_idx].size());

	for (const double angle : goal_angles[cur_goal_idx]) {
		ends.emplace_back(goals[cur_goal_idx], angle);
	}

	return ends;
}

bool RRT::connectToGoal(int cur_goal_idx) {
	// TODO : max_paths_checked should be rearchitected
	const Connection connection =
		bestConnection(goalEndpoints(cur_goal_idx), std::numeric_limits<int>::max());

	if (!connection.isValid()) {
		return false;
	}

	commitConnection(connection, cur_goal_idx);
	return true;
}

void RRT::commitConnection(const Connection& connection, int cur_goal_idx) {
    const NodeId goal_node = tree.tree.size;
	tree.addSample(connection.anchor, connection.end, connection.option);

	/* The dubins paths this leg settled on are pulled out of the tree before it is
	 * thrown away. The points along them are not generated here -- the mission may
	 * yet be dropped for a cheaper one, and its length is known without them.
	 */
	legs.push_back(
		{tree.getStart(), tree.findPathToNode(goal_node), connection.cost, cur_goal_idx});

	// the goal becomes the root of a fresh tree for the next waypoint
	tree.setCurrentHead(connection.end);
}

std::vector<XYZCoord> RRT::buildFlightPath(const Leg& leg) const {
	std::vector<XYZCoord> path = Dubins::generatePath(leg.start, leg.segments);

	if (path.empty()) {
		return path;
	}

	// the leg is flown from the waypoint behind the one it lands on
	const double start_height = goals[leg.goal_idx - 1].z;
	const double height_difference = goals[leg.goal_idx].z - start_height;

	/* The plane climbs at a constant rate, so a point's altitude depends on how far along
	 * the ground it is, not on how many points came before it. The points are not evenly
	 * spaced (straightaways only have their two endpoints), so each one carries the distance
	 * flown from the start of the leg to reach it until the total is known and it can be
	 * turned into an altitude.
	 */
	XYZCoord previous = leg.start.coord;
	double total_distance = 0;

	for (XYZCoord& point : path) {
		total_distance += std::hypot(point.x - previous.x, point.y - previous.y);
		previous = point;
		point.z = total_distance;
	}

	for (XYZCoord& point : path) {
		// a path that covers no ground puts every point at the goal altitude
		const double ratio = (total_distance > 0) ? point.z / total_distance : 1.0;
		point.z = start_height + height_difference * ratio;
	}

	return path;
}
