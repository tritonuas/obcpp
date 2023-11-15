#ifndef NETWORK_GCS_HPP
#define NETWORK_GCS_HPP

#include <thread>
#include <memory>

#include "core/config.hpp"
#include "core/states.hpp"

class GCSServer {
    public:
        GCSServer(
            std::shared_ptr<MissionConfig> config,
            std::shared_ptr<std::unique_ptr<MissionState>> state
        );

        ~GCSServer();
    private:
        std::shared_ptr<MissionConfig> config;
        std::shared_ptr<std::unique_ptr<MissionState>> state;

        std::thread server_thread;

        // Called inside constructor in its own thread
        void _listen();
};

#endif // NETWORK_GCS_HPP