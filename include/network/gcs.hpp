#ifndef NETWORK_GCS_HPP
#define NETWORK_GCS_HPP

#include <thread>
#include <memory>
#include <mutex>
#include <cstdint>

#include <httplib.h>

#include "core/config.hpp"
#include "core/states.hpp"

class GCSServer {
    public:
        GCSServer(uint16_t port, std::shared_ptr<MissionState> state);

        ~GCSServer();
    private:
        httplib::Server server;
        std::mutex server_mut;
        std::thread server_thread;
        uint16_t port;

        std::shared_ptr<MissionState> state;

        // Handler Functions
        void _bindHandlers(); // bind all the handlers to the server object
        void _getMission(const httplib::Request&, httplib::Response&);
};

#endif // NETWORK_GCS_HPP