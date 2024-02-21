#include "network/airdrop_client.hpp"

extern "C" {
    #include "network/airdrop_sockets.h"
}

AirdropClient::AirdropClient() {
    ad_socket_result_t res = make_ad_socket(AD_OBC_PORT, AD_PAYLOAD_PORT);
}