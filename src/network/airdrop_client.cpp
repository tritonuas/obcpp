#include "network/airdrop_client.hpp"

extern "C" {
    #include "airdrop/packet.h"
    #include "network/airdrop_sockets.h"
}

#include "utilities/locks.hpp"
#include "utilities/logging.hpp"

AirdropClient::AirdropClient(ad_socket_t socket) {
    this->socket = socket;
    // TODO: establish mode
}

AirdropClient::~AirdropClient() {

}

bool AirdropClient::send(ad_packet_t packet) {
    set_send_thread();
    auto res = send_ad_packet(this->socket, &packet);
    if (res.is_err) {
        LOG_F(ERROR, "%s", res.data.err);
        return false;
    }

    return true;
}

std::optional<ad_packet_t> AirdropClient::receive() {
    Lock lock(this->recv_mut);

    if (this->recv_queue.empty()) {
        return {};
    }

    auto packet = this->recv_queue.front();
    this->recv_queue.pop();
    return packet;
}

void AirdropClient::receiveWorker() {
    set_recv_thread();

    ad_packet_t* packet_buf = new ad_packet_t;

    while (true) {
        auto result = recv_ad_packet(this->socket, 
            static_cast<void*>(packet_buf),
            sizeof(ad_packet_t));
        // block until packet found...

        if (result.is_err) {
            LOG_F(ERROR, "%s", result.data.err);
            continue;
        }

        Lock lock (this->recv_mut);
        this->recv_queue.emplace(ad_packet_t { .hdr = packet_buf->hdr, .data = packet_buf->data, })
    }
}