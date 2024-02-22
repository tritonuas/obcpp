#ifndef INCLUDE_NETWORK_AIRDROP_HPP_
#define INCLUDE_NETWORK_AIRDROP_HPP_

extern "C" {
    #include "airdrop/packet.h"
    #include "network/airdrop_sockets.h"
}

#include <optional>
#include <queue>
#include <mutex>
#include <atomic>

#include "protos/obc.pb.h"

class AirdropClient {
 public:
    // Blocking until we receive SET_MODE packets from the payloads
    AirdropClient(ad_socket_t socket);
    ~AirdropClient();

    void establishConnection();
    bool isConnectionEstablished() const;

    bool send(ad_packet_t packet);
    std::optional<ad_packet_t> receive();

 private:
    std::optional<ad_mode> mode {};
    ad_socket_t socket {};

    std::queue<ad_packet_t> recv_queue;
    std::mutex recv_mut;

    std::atomic_bool stopWorker;
    std::future<void> workerFuture;

    // Function to run in its own thread
    void _receiveWorker();
    ad_packet_t _receiveBlocking();
};

#endif  // INCLUDE_NETWORK_AIRDROP_HPP_
