#include "network/airdrop_client.hpp"

#include <future>

extern "C" {
    #include "airdrop/packet.h"
    #include "network/airdrop_sockets.h"
}
#include "utilities/locks.hpp"
#include "utilities/logging.hpp"
#include "utilities/common.hpp"

AirdropClient::AirdropClient(ad_socket_t socket) {
    this->socket = socket;
    auto time = getUnixTime_ms();
    for (int curr_bottle = BOTTLE_A; curr_bottle <= BOTTLE_E; curr_bottle++) {
        this->last_heartbeat[curr_bottle - 1] = time;
    }
    this->_establishConnection(); // block until connection established
}

AirdropClient::~AirdropClient() {
    // Signal before killing the socket so when the worker gets kicked out of the socket
    // it sees that it should stop.
    this->stop_worker = true;
    LOG_F(INFO, "Attempting to destroy AirdropClient");

    auto result = close_ad_socket(this->socket);
    if (result.is_err) {
        LOG_F(ERROR, "Error closing airdrop socket: %s", result.data.err);
    }

    if (this->mode.has_value()) {
        // the future was started so we should make sure it is stopped
        LOG_F(INFO, "Waiting on background receive thread to exit...");
        this->worker_future.get();
        LOG_F(INFO, "Background receive thread exited");
    }
}

void AirdropClient::_establishConnection() {
    LOG_F(INFO, "Attempting to establish connection with the payloads...");

    while (true) {
        auto packet = this->_receiveBlocking();
        if (validate_packet_as(SET_MODE, packet)) {
            this->mode = static_cast<ad_mode>(packet.data);
            break;
        }

        LOG_F(WARNING, "Non SET_MODE packet received in setup phase: %d %d",
            packet.hdr, packet.data);
    }

    LOG_F(INFO, "Payload connection established in %s mode",
        (this->mode == DIRECT_DROP) ? "Direct" : "Indirect");

    this->worker_future = std::async(std::launch::async, &AirdropClient::_receiveWorker, this);
}

bool AirdropClient::send(ad_packet_t packet) {
    set_send_thread();

    auto res = send_ad_packet(this->socket, packet);
    if (res.is_err) {
        LOG_F(ERROR, "%s", res.data.err);
        return false;
    }

    // TODO: helper to go from packet -> str
    LOG_F(INFO, "Sent airdrop packet: %hhu %hhu", packet.hdr, packet.data);
    return true;
}

std::optional<ad_packet_t> AirdropClient::receive() {
    Lock lock(this->recv_mut);

    if (this->recv_queue.empty()) {
        return {};
    }

    auto packet = this->recv_queue.front();
    this->recv_queue.pop();
    LOG_F(INFO, "Pulled packet from queue: %hhu %hhu", packet.hdr, packet.data);
    return packet;
}

std::list<std::pair<BottleDropIndex, std::chrono::milliseconds>>
    AirdropClient::getLostConnections(std::chrono::milliseconds threshold) {
    std::list<std::pair<BottleDropIndex, std::chrono::milliseconds>> list;
    auto time = getUnixTime_ms();

    for (int i = 0; i < this->last_heartbeat.size(); i++) {
        auto time_since_last_heartbeat = time - this->last_heartbeat[i];
        if (time_since_last_heartbeat >= threshold) {
            list.push_back({static_cast<BottleDropIndex>(i + 1), time_since_last_heartbeat});
        }
    }

    return list;
}

std::optional<ad_mode> AirdropClient::getMode() {
    return this->mode;
}

ad_packet_t AirdropClient::_receiveBlocking() {
    ad_packet_t packet = { 0 };

    while (true) {
        VLOG_F(TRACE, "Airdrop worker waiting for airdrop packet...");
        auto result = recv_ad_packet(this->socket,
            static_cast<void*>(&packet),
            sizeof(ad_packet_t));
        VLOG_F(TRACE, "Airdrop packet found...");
        // block until packet found...

        // in the destructor we close the socket. This will cause the worker thread
        // to get kicked out of the recv call with an error. This is the only error
        // that should cause this function to return an invalid packet.

        if (this->stop_worker) {
            return packet;  // dummy packet
        }

        if (result.is_err) {
            LOG_F(ERROR, "%s", result.data.err);
            continue;
        }

        // Should return the number of bytes read, which should be equal to the
        // size of a packet. If this happens, we probably just read in some
        // garbage data that shouldn't have been read by this program.
        if (result.data.res != sizeof(ad_packet_t)) {
            LOG_F(ERROR, "recv read %d bytes, when a packet should be %lu. Ignoring...",
                result.data.res, sizeof(ad_packet_t));
            continue;
        }

        // TODO: helper to go from packet -> str
        VLOG_F(TRACE, "received airdrop packet: %hhu %hhu", packet.hdr, packet.data);

        return packet;
    }
}

void AirdropClient::_receiveWorker() {
    set_recv_thread();

    while (true) {
        ad_packet_t packet = this->_receiveBlocking();

        if (this->stop_worker) {
            return;  // kill worker :o
        }

        if (this->_parseHeartbeats(packet)) {
            continue;  // heartbeat, so we should not put it in the queue
        }

        Lock lock(this->recv_mut);
        this->recv_queue.emplace(packet);
    }
}

bool AirdropClient::_parseHeartbeats(ad_packet_t packet) {
    if (!validate_packet_as(HEARTBEAT, packet)) {
        return false;
    }

    // Valid heartbeat packet, so packet.data is within
    // [1, 5] and corresponds to a bottle index, so we can
    // subtract 1 to get the index into the lastHeartbeat array.
    this->last_heartbeat[packet.data - 1] = getUnixTime_ms();

    return true;
}
