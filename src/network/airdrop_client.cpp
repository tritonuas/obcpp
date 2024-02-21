#include "network/airdrop_client.hpp"

extern "C" {
    #include "airdrop/packet.h"
    #include "network/airdrop_sockets.h"
}

#include "utilities/logging.hpp"

AirdropClient::AirdropClient(ad_socket_t socket) {
    this->socket = socket;
}

bool AirdropClient::send(ad_packet_t packet) {
    auto res = send_ad_packet(this->socket, &packet);
    if (res.is_err) {
        // log error
    }
}

std::optional<ad_packet_t> AirdropClient::receive() {
    
}

void AirdropClient::receiveWorker() {

}