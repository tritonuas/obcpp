#ifndef INCLUDE_NETWORK_AIRDROP_CLIENT_HPP_
#define INCLUDE_NETWORK_AIRDROP_CLIENT_HPP_

#include <optional>
#include <queue>
#include <mutex>
#include <atomic>
#include <array>
#include <list>
#include <utility>
#include <future>

extern "C" {
    #include "airdrop/packet.h"
    #include "network/airdrop_sockets.h"
}
#include "protos/obc.pb.h"
#include "utilities/constants.hpp"

class AirdropClient {
 public:
    // Blocking until we receive SET_MODE packets from the payloads
    explicit AirdropClient(ad_socket_t socket);
    ~AirdropClient();

    void establishConnection();
    bool isConnectionEstablished() const;

    bool send(ad_packet_t packet);
    // Receives oldest packet since last receive() call, ignoring any
    // HEARTBEAT packets as those are parsed by the client itself
    // and exposed through the TODO function.
    std::optional<ad_packet_t> receive();

    // Returns list of all the payloads we have not heard from for more than
    // `threshold` seconds, and includes how many seconds it has been since
    // we last heard from them.
    std::list<std::pair<BottleDropIndex, std::chrono::milliseconds>>
        getLostConnections(std::chrono::milliseconds threshold);

    std::optional<ad_mode> getMode();

 private:
    std::optional<ad_mode> mode {};
    ad_socket_t socket {};

    std::queue<ad_packet_t> recv_queue;
    std::mutex recv_mut;

    std::atomic_bool stop_worker;
    std::future<void> worker_future;

    // holds unix timestamp of the last heartbeat received from every payload
    std::array<std::chrono::milliseconds, NUM_AIRDROP_BOTTLES> last_heartbeat;

    // Function to run in its own thread
    void _receiveWorker();

    // Blocking implementation of receive for internal use
    ad_packet_t _receiveBlocking();

    // Parses packets for heartbeat information and stores it in the
    // lastHeartbeat array.
    // Returns true if it is a heartbeat packet, in which case
    // it should NOT be placed in the recv_queue. Otherwise, returns
    // false, meaning that it was NOT a heartbeat and should be exposed
    // to the user of the client through the nonblocking receive function.
    bool _parseHeartbeats(ad_packet_t packet);
};

#endif  // INCLUDE_NETWORK_AIRDROP_CLIENT_HPP_
