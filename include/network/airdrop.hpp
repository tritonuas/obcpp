#ifndef INCLUDE_NETWORK_AIRDROP_HPP_
#define INCLUDE_NETWORK_AIRDROP_HPP_

#include "protos/obc.pb.h"

class AirdropClient {
 public:
    AirdropClient();

    /*
     * Signals that a bottle is ready to drop
     *
     */
    void signal(BottleDropIndex bottle) const;
};

#endif  // INCLUDE_NETWORK_AIRDROP_HPP_
