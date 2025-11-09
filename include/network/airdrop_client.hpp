#ifndef INCLUDE_NETWORK_AIRDROP_CLIENT_HPP_
#define INCLUDE_NETWORK_AIRDROP_CLIENT_HPP_

#include <array>
#include <atomic>
#include <future>
#include <list>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

extern "C" {
#include "network/airdrop_sockets.h"
#include "udp_squared/protocol.h"
}
#include "protos/obc.pb.h"
#include "utilities/constants.hpp"

class AirdropClient {
 public:
    // Blocking until we receive SET_MODE packets from the payloads
    explicit AirdropClient(ad_socket_t socket);
    ~AirdropClient();

    bool send(packet_t packet);
    // Receives oldest packet since last receive() call, ignoring any
    // HEARTBEAT packets as those are parsed by the client itself
    // and exposed through the TODO function.
    std::optional<packet_t> receive();

    // Returns list of all the payloads we have not heard from for more than
    // `threshold` seconds, and includes how many seconds it has been since
    // we last heard from them.
    std::list<std::pair<AirdropType, std::chrono::milliseconds>> getLostConnections(
        std::chrono::milliseconds threshold);

    std::optional<drop_mode_t> getMode();

 private:
    std::optional<drop_mode_t> mode{};
    ad_socket_t socket{};

    std::queue<packet_t> recv_queue;
    std::mutex recv_mut;

    std::atomic_bool stop_worker;
    std::future<void> worker_future;

    // holds unix timestamp of the last heartbeat received from every payload
    std::array<std::chrono::milliseconds, NUM_AIRDROPS> last_heartbeat;

    // Get initial SET_MODE msg from payloads and set up workers
    void _establishConnection();

    // Function to run in its own thread
    void _receiveWorker();

    // Blocking implementation of receive for internal use
    packet_t _receiveBlocking();

    // Parses packets for heartbeat information and stores it in the
    // lastHeartbeat array.
    // Returns true if it is a heartbeat packet, in which case
    // it should NOT be placed in the recv_queue. Otherwise, returns
    // false, meaning that it was NOT a heartbeat and should be exposed
    // to the user of the client through the nonblocking receive function.
    bool _parseHeartbeats(packet_t packet);
};

#endif  // INCLUDE_NETWORK_AIRDROP_CLIENT_HPP_
