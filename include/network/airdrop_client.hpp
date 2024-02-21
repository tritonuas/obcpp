#ifndef INCLUDE_NETWORK_AIRDROP_HPP_
#define INCLUDE_NETWORK_AIRDROP_HPP_

extern "C" {
    #include "airdrop/packet.h"
    #include "network/airdrop_sockets.h"
}

#include <optional>

#include "protos/obc.pb.h"

class AirdropClient {
 public:
    AirdropClient();

 private:
    std::optional<ad_mode> mode {};
    int socket {};
};

#endif  // INCLUDE_NETWORK_AIRDROP_HPP_
