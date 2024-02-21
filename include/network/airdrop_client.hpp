#ifndef INCLUDE_NETWORK_AIRDROP_HPP_
#define INCLUDE_NETWORK_AIRDROP_HPP_

extern "C" {
    #include "airdrop/packet.h"
    #include "network/airdrop_sockets.h"
}

#include <optional>
#include <deque>
#include <mutex>
#include <condition_variable>

#include "protos/obc.pb.h"

class AirdropClient {
 public:
    AirdropClient(ad_socket_t socket);

    void send(ad_packet_t packet);
    std::optional<ad_packet_t> receive();

 private:
    std::optional<ad_mode> mode {};
    ad_socket_t socket {};

    std::deque<ad_packet_t> recv_deque;
    std::mutex recv_mut;

    // Functions to run in their own thread
    void sendWorker();
    void receiveWorker();
};

#endif  // INCLUDE_NETWORK_AIRDROP_HPP_
