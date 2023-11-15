#ifndef CORE_CONFIG_HPP
#define CORE_CONFIG_HPP

#include <array>
#include <shared_mutex>
#include <optional>
#include <unordered_map>

#include "utilities/datatypes.hpp"
#include "utilities/constants.hpp"


/*
 *  Thread-safe wrapper around the Mission Configuration options.
 *  Multiple threads can access this same object, and it will handle
 *  all memory accesses with the shared mutex
 */
class MissionConfig {
    public:
        MissionConfig(); // Load default values
        MissionConfig(std::string filename); // Load from filename

        // Getters for singular value
        // Use when only need to read one value
        Polygon getFlightBoundary();        
        Polygon getAirdropBoundary();
        Polyline getWaypoints();
        BottleArray getAirdropBottles(); 

        // Getters for multiple values
        // Use when need to get multiple values
        // Important to use this instead of the singular getters
        // to avoid race conditions
        std::tuple<Polygon, Polygon, Polyline, BottleArray> getConfig();

        // Setters for singular value
        // Use when only need to update one value
        void setFlightBoundary(Polygon bound);
        void setAirdropBoundary(Polygon bound);
        void setWaypoints(Polyline wpts);
        // label must be a letter from 'A' to 'F', lowercase is also accepted
        void setBottle(char label, CompetitionBottle bottle);
        // Update multiple bottles at a time
        void setBottles(const std::unordered_map<char, CompetitionBottle>& bottleUpdates);

        // Use when need to update many things at once
        void update(
            std::optional<Polygon> flight,
            std::optional<Polygon> airdrop,
            std::optional<Polyline> waypoints,
            std::unordered_map<char, CompetitionBottle> bottleUpdates
        );

        void saveToFile(std::string filename);
    private:
        std::shared_mutex mut;

        Polygon flightBoundary;
        Polygon airdropBoundary;
        Polyline waypoints;
        BottleArray bottles;

        // internal function which doesn't apply the mutex
        // abstracts the logic checking the char label
        void _setBottle(char label, CompetitionBottle bottle);
};

#endif