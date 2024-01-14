/*
    State for when the system has just been turned on and is waiting for
    mission parameters.
*/
class PreparationState: public MissionState {
    public:
        ~PreparationState() override = default;
        MissionState* tick() override;

        std::string getName() override {
            return "Mission Preparation";
        }

    private:
        std::optional<Polygon> flightBoundary;
        std::optional<Polygon> airdropBoundary;
        std::optional<Polyline> waypoints;
        std::array<CompetitionBottle, NUM_AIRDROP_BOTTLES> bottles;
};


/*
    State for when plane is actively evading a dynamic obstacle.
    Plane enters this state after a *dynamic* obstacle is detected. This state
    will choose a form of active evasion and then return control to normal
    pathing afterwards.
    The path planning does not run in real-time, so avoidance behaves by inserting
    emergency waypoints and then returning to continue the next unhit waypoint.
*/
class ObstacleEvasionState: public MissionState {
    public:
        // Do not use a default constructor. The evasion state needs to know
        // the location of the obstacle
        ObstacleEvasionState() override = delete;
        // List of obstacles to avoid
        ObstacleEvasionState(std::vector<GPSCoord> obstacles);
        ~ObstacleEvasionState() override = default;

        MissionState* tick() override;

        std::string getName() override {
            return "Evading Dynamic Obstacle";
        }
        
        void updateObstaclesList(std::vector<GPSCoord> obstacles);

    private:
        std::vector<GPSCoord> obstacles;
        /*
            Do the evasion logic and send mavlink commands.
        */
        void evade(void);
};
