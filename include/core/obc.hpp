#ifndef CORE_OBC_HPP
#define CORE_OBC_HPP

#include <string>
#include <memory>

#include "core/config.hpp"
#include "core/states.hpp"

#include "network/gcs.hpp"

/*
 *  The highest level class for the entire OBC
 *  This contains all of the mission state, mission config, etc...
 */
class OBC {
    public:
        OBC();

        void run();

    private:
        std::shared_ptr<MissionState> state;

        std::unique_ptr<GCSServer> gcs_server;
};

#endif // CORE_OBC_HPP