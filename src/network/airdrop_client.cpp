#include "network/airdrop_client.hpp"

extern "C" {
    #include "airdrop/packet.h"
    #include "network/airdrop_sockets.h"
}

#include <future>

#include "utilities/locks.hpp"
#include "utilities/logging.hpp"
#include "airdrop_client.hpp"

AirdropClient::AirdropClient(ad_socket_t socket) {
    this->socket = socket;
    set_send_thread(); // send from same thread as the client is created in
}

AirdropClient::~AirdropClient() {
    // Signal before killing the socket so when the worker gets kicked out of the socket
    // it sees that it should stop.
    this->stopWorker = true;

    auto result = close_ad_socket(this->socket);
    if (result.is_err) {
        LOG_F(ERROR, "~AirdropClient() %s", result.data.err);
    }

    if (this->mode.has_value()) {
        // the future was started so we should make sure it is stopped
        this->workerFuture.get();
    }
}

void AirdropClient::establishConnection() {
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

    this->workerFuture = std::async(std::launch::async, &AirdropClient::_receiveWorker, this);
}

bool AirdropClient::isConnectionEstablished() const {
    return this->mode.has_value();
}

bool AirdropClient::send(ad_packet_t packet) {
    auto res = send_ad_packet(this->socket, &packet);
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

ad_packet_t AirdropClient::_receiveBlocking() {
    ad_packet_t packet = { 0 };

    while (true) {
        auto result = recv_ad_packet(this->socket,
            static_cast<void*>(&packet),
            sizeof(ad_packet_t));
        // block until packet found...

        // in the destructor we close the socket. This will cause the worker thread
        // to get kicked out of the recv call with an error. This is the only error
        // that should cause this function to return an invalid packet.

        if (this->stopWorker) {
            return packet; // dummy packet
        }

        if (result.is_err) {
            LOG_F(ERROR, "%s", result.data.err);
            continue;
        }

        // Should return the number of bytes read, which should be equal to the
        // size of a packet. If this happens, we probably just read in some
        // garbage data that shouldn't have been read by this program.
        if (result.data.res != sizeof(ad_packet_t)) {
            LOG_F(ERROR, "recv read %d bytes, when a packet should be %d. Ignoring...", 
                result.data.res, sizeof(ad_packet_t));
            continue;
        }

        // TODO: helper to go from packet -> str
        LOG_F(INFO, "received airdrop packet: %hhu %hhu", packet.hdr, packet.data);
        return packet;
    }
}

void AirdropClient::_receiveWorker() {
    set_recv_thread();

    while (true) {
        ad_packet_t packet = this->_receiveBlocking();

        if (this->stopWorker) {
            return; // kill worker :o
        }

        Lock lock (this->recv_mut);
        this->recv_queue.emplace(packet);
    }
}