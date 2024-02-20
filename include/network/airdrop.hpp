#ifndef INCLUDE_AIRDROP_CONTROLS_HPP_
#define INCLUDE_AIRDROP_CONTROLS_HPP_

#include "obc.pb.h"

class AirdropClient {
 public:
    AirdropClient();

    /*
     * Signals that a bottle is ready to drop
     *
     */
    void signal(BottleDropIndex bottle) const;
 private:

};

#endif  // INCLUDE_AIRDROP_CONTROLS_HPP_
