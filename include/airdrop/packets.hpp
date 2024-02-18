#include <cstdint>

/*
 * This file defines the packet definitions for Airdrop communications. It is used in two
 * separate repositories: 
 * 
 * 1. https://github.com/tritonuas/guided-payload
 * 2. https://github.com/tritonuas/obcpp
 * 
 * If you need to make a change to this file, you will have to ensure that it persists across
 * both repositories. We could set up a git submodule to make this "better" in that we only
 * have one source of truth for this file, which both repos pull from, but right now we are
 * assuming that will be more trouble that it is worth.
 */

enum AirdropPacketType {
    SIGNAL  = 0,  // OBC tells a bottle it is cleared to drop
    REVOKE  = 1,  // OBC tells a bottle it is NOT cleared to drop. Overwrites any previous SIGNAL.
    DROPPED = 2,  // Bottle tells OBC that it has dropped
};

// All of the values in here are 1 byte long, which means we do not have to worry about
// the endianness of the information as it is sent over the wire. If we ever add values more
// than one byte long, we'll have to worry about this.

struct __attribute__ ((packed)) AirdropPacket {
    uint8_t type;    // use AirdropPacketType as defined above
    uint8_t bottle;  // use BottleDropIndex defined in protobuf
};