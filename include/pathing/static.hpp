#ifndef INCLUDE_PATHING_STATIC_HPP_
#define INCLUDE_PATHING_STATIC_HPP_

#include <httplib.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "core/mission_state.hpp"
#include "pathing/dubins.hpp"
#include "pathing/environment.hpp"
#include "pathing/plotting.hpp"
#include "pathing/tree.hpp"
#include "utilities/constants.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/rng.hpp"

class RRT {
public:
  RRT(RRTPoint start, std::vector<XYZCoord> goals, int iterations_per_waypoint,
      double search_radius, double rewire_radius, Polygon bounds,
      std::vector<Polygon> obstacles = {},
      OptimizationOptions options = {true, PATH_OPTIONS::NONE})

      : iterations_per_waypoint(iterations_per_waypoint),
        search_radius(search_radius), rewire_radius(rewire_radius),

        point_fetch_choice(options.path_option),
        tree(start, Environment(bounds, goals, obstacles),
             Dubins(TURNING_RADIUS, POINT_SEPARATION)),
        optimize(options.optimize){};

  /**
   * RRT algorithm
   * - Treats each waypoint as a goal, DOES NOT reuse trees between waypoints,
   *    basically calls RRT for each waypoint
   * - For Each Waypoint
   *  - Tries to connect directly to the goal
   *  - If it can't, it runs the RRT algorithm
   *      - Attempts to converge based on epoch intervals
   *      - If it can't, it connects to the goal with whatever it has
   *
   * TODO - do all iterations to try to find the most efficient path?
   *  - maybe do the tolarance as stright distance / num iterations
   *  - not literally that function, but something that gets more leniant the
   * more iterations there are
   */
  void run() {
    const int total_goals = tree.getAirspace().getNumGoals();

    for (int current_goal_index = 0; current_goal_index < total_goals;
         current_goal_index++) {
      // tries to connect directly to the goal
      if (connectToGoal(current_goal_index)) {
        continue;
      }

      // run the RRT algorithm if it can not connect
      RRTIteration(iterations_per_waypoint, current_goal_index);
    }
  }

  /**
   * returns a continuous path of points to the goal
   *
   * @return  ==> list of 2-vectors to the goal region
   */
  std::vector<XYZCoord> getPointsToGoal() { return tree.getPathToGoal(); }

private:
  // tree stores
  // - the airspace (environment.hpp) that checks valid bounds
  // - the dubins object that generates paths
  // - the nodes that that form the tree
  RRTTree tree;

  /* RRT Config Options */
  const bool optimize;               // true if RRT-Star is to be used
  const int iterations_per_waypoint; // number of times to run the RRT algorithm
                                     // for each waypoint
  const double search_radius; // !!NOT USED!! max radius to move off the tree
  const double
      rewire_radius; // ONLY FOR RRT-STAR, max radius from new node to rewire
  const PATH_OPTIONS point_fetch_choice; // how to select points for RRT (not
                                         // -Star) NONE: optimal-every_point,
                                         // RANDOM: fastest-random,
                                         // NEAREST: mix-nearest

  // the different of final approaches to the goal
  // yes, this is the default unit circle diagram used in High-School
  const std::vector<double> angles = {
      0,
      M_PI / 6,
      M_PI / 4,
      M_PI / 3,
      M_PI / 2,
      2 * M_PI / 3,
      3 * M_PI / 4,
      5 * M_PI / 6,
      M_PI,
      7 * M_PI / 6,
      5 * M_PI / 4,
      4 * M_PI / 3,
      3 * M_PI / 2,
      5 * M_PI / 3,
      7 * M_PI / 4,
      11 * M_PI / 6,
  };

  /**
   * Does a single iteration of the RRT(star) algoritm to connect two waypoints
   *
   * @param tries ==> number of points it attempts to sample
   */
  void RRTIteration(const int tries, const int current_goal_index) {
    int epoch_interval = tries / 5;
    int current_epoch = epoch_interval;

    RRTNode *goal_node = nullptr;

    for (int i = 0; i < tries; i++) {
      if (i == current_epoch) {
        // generates a new node (not connect), and adds and breaks if it is
        // within X% of the last generation
        if (epochEvaluation(goal_node, current_goal_index)) {
          return;
        }

        current_epoch += epoch_interval;
      }
      // generate a sample point
      const RRTPoint sample = generateSamplePoint();

      // returns all dubins options from the tree to the sample
      const std::vector<std::pair<RRTNode *, RRTOption>> &options =
          tree.pathingOptions(sample, point_fetch_choice);

      // returns true if the node is successfully added to the tree
      RRTNode *new_node = parseOptions(options, sample);

      if (new_node != nullptr && optimize) {
        optimizeTree(new_node);
      }
    }

    // frees memory
    delete (goal_node);
    connectToGoal(current_goal_index);
  }

  /**
   * Evaluates a certain interval to determine if the algorithm is making
   * meaningful progress. If it isn't, it will simply tell the RRT algoritm to
   * stop.
   */
  bool epochEvaluation(RRTNode *goal_node, int current_goal_index) {
    // If a single epoch has not been passed, mark this goal as the first
    // benchmark.
    if (goal_node == nullptr) {
      goal_node = sampleToGoal(current_goal_index);
      return false;
    }

    RRTNode *new_node = sampleToGoal(current_goal_index);

    if (new_node == nullptr) {
      return false;
    }

    delete (goal_node);
    goal_node = new_node;

    // If the new node is within ~2% of the goal, then we are done.
    // It should be impossible for new_node to be more inefficient than
    // goal_node as it uses a superset of the tree goal_node used
    if (new_node->getCost() < 0.98 * goal_node->getCost()) {
      return false;
    }

    tree.addNode(new_node->getParent(), new_node);
    tree.setCurrentHead(new_node);
    return true;
  }

  /**
   * Generates a random point in the airspace (uniformly)
   *
   * @return  ==> random point in the airspace
   */
  RRTPoint generateSamplePoint() { return tree.getRandomPoint(search_radius); }

  /**
   * Gets a sorted list of options to EACH one of the possible goals, defined
   * by the angles we want to connect to
   *
   * @param current_goal_index    ==> index of the goal that we are trying to
   * connect to
   * @return                      ==> list of options to connect to the goal
   *                                  <RRTPoint GOAL, {RRTNode* ANCHOR,
   * RRTOption} >
   */
  std::vector<std::pair<RRTPoint, std::pair<RRTNode *, RRTOption>>>
  getOptionsToGoal(int current_goal_index) const {
    // attempts to connect to the goal, should always connect
    std::vector<RRTPoint> goal_points;

    // Generates goal specific points based on current Waypoints and list og
    // Angles
    for (const double angle : angles) {
      const XYZCoord goal = tree.getAirspace().getGoal(current_goal_index);
      goal_points.push_back(RRTPoint(goal, angle));
    }

    // RRTPoint is the goal that is to be connected
    // RRTNode is the node in the tree that is the anchor
    // RRTOPtion Node-->Point
    std::vector<std::pair<RRTPoint, std::pair<RRTNode *, RRTOption>>>
        all_options;

    // limit amount of options to sort, defined in constants.hpp
    const int NUMBER_OPTIONS_EACH =
        TOTAL_OPTIONS_FOR_GOAL_CONNECTION / angles.size();

    // gets all options for each of the goals, and puts them into a unified list
    // TODO ? maybe better for a max heap?
    for (const RRTPoint &goal : goal_points) {
      const std::vector<std::pair<RRTNode *, RRTOption>> &options =
          // For now, we use optimal pathing
          tree.pathingOptions(goal, PATH_OPTIONS::NONE, NUMBER_OPTIONS_EACH);

      for (const auto &[node, option] : options) {
        all_options.push_back({goal, {node, option}});
      }
    }

    std::sort(all_options.begin(), all_options.end(),
              [](const auto &a, const auto &b) {
                auto &[a_goal, a_paths] = a;
                auto &[a_node, a_option] = a_paths;
                auto &[b_goal, b_paths] = b;
                auto &[b_node, b_option] = b_paths;
                return a_option.length + a_node->getCost() <
                       b_option.length + b_node->getCost();
              });

    return all_options;
  }

  /**
   * Tries to get the optimal  node to the goal, which is NOT connected into the
   * tree
   *
   * @param current_goal_index    ==> index of the goal that we are trying to
   * connect to
   * @return                      ==> pointer to the node if one was found,
   * nullptr otherwise
   */
  RRTNode *sampleToGoal(int current_goal_index) {
    // gets all options for each of the goals
    const std::vector<std::pair<RRTPoint, std::pair<RRTNode *, RRTOption>>>
        &all_options = getOptionsToGoal(current_goal_index);

    // <RRTPoint GOAL, {RRTNode* ANCHOR, RRTOption} >
    for (const auto &[goal, pair] : all_options) {
      auto &[anchor_node, option] = pair;

      RRTNode *new_node = tree.generateNode(anchor_node, goal, option);

      if (new_node != nullptr) {
        return new_node;
      }
    }

    return nullptr;
  }

  /**
   * Connects to the goal after RRT is finished
   *
   * @param current_goal_index    ==> index of the goal that we are trying to
   * connect to
   * @return                      ==> pointer to the node if it was added,
   * nullptr otherwise
   */
  bool connectToGoal(int current_goal_index) {
    RRTNode *goal_node = sampleToGoal(current_goal_index);

    if (goal_node == nullptr) {
      return false;
    }

    // sets the new head
    tree.addNode(goal_node->getParent(), goal_node);
    tree.setCurrentHead(goal_node);
    return true;
  }

  /**
   * Goes through generated options to try to connect the sample to the tree
   *
   * @param options   ==> list of options to connect the sample to the tree
   * @param sample    ==> sampled point
   * @return          ==> whether or not the sample was successfully added to
   * the tree (nullptr if not added)
   */
  RRTNode *
  parseOptions(const std::vector<std::pair<RRTNode *, RRTOption>> &options,
               const RRTPoint &sample) {
    // directly putting options.size inside the min function angers the
    // compiler??? I guess it doesn't know options.size() at compile time?
    for (auto &[node, option] : options) {
      /*
       *  stop if
       *  1. the node is null
       *  2. the node is the same as the sample
       *
       *  The idea is that any further options will have the same if not more
       * issues
       *
       * This shouldn't ever happen?
       */
      // if (node == nullptr || node->getPoint() == sample) {
      //     return nullptr;
      // }

      // else, attempt to add the node to the tree
      RRTNode *sucessful_addition = tree.addSample(node, sample, option);

      if (sucessful_addition != nullptr) {
        return sucessful_addition;
      }
    }

    return nullptr;
  }

  /**
   * Rewires the tree by finding paths that are more efficintly routed through
   * the sample. Only searches for nodes a specific radius around the sample
   * to reduce computational expense
   *
   * @param sample    ==> sampled point
   */
  void optimizeTree(RRTNode *sample) { tree.RRTStar(sample, rewire_radius); }
};

std::vector<GPSCoord> generateInitialPath(std::shared_ptr<MissionState> state);

#endif // INCLUDE_PATHING_STATIC_HPP_
