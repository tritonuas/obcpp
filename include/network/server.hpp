#ifndef NETWORK_SERVER_HPP_
#define NETWORK_SERVER_HPP_

#include <memory>

#include "core/config.hpp"

/*
 *  The highest level class for the entire OBC
 *  This contains all of the mission state, mission config, etc...
 */
class OBCServer {
    public:
        OBCServer(std::string config);

    private:
        std::shared_ptr<MissionConfig> config;
        std::shared_ptr<MissionState> state;
}

#endif // NETWORK_SERVER_HPP_