#ifndef INCLUDE_AIRDROP_PACKET_H_
#define INCLUDE_AIRDROP_PACKET_H_

#include <stdint.h>

/**********************************************************************************************
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
 * 
 ***********************************************************************************************
 *
 * Types of packets and what they mean
 * 
 * TODO: write documentation 
 */

#define AD_OBC_PORT     45906  // OBC listens on this port, so payloads send to this
#define AD_PAYLOAD_PORT 45907  // Payloads listen on this port, so OBC sends to this

// All of the values in here are 1 byte long, which means we do not have to worry about
// the endianness of the information as it is sent over the wire. If we ever add values more
// than one byte long, we'll have to worry about this.

// gcc specific packing syntax. Technically this should already be packed because it is just
// two one-byte values, but just making sure.
struct __attribute__((packed)) ad_packet {
    uint8_t hdr;
    uint8_t data;
};
typedef struct ad_packet ad_packet_t;

enum ad_packet_hdr {
    // Continually sent by the payloads to confirm they are still operational
    // and can talk to the OBC
    HEARTBEAT   = 0,

    // Handshake to establish connection
    SET_MODE    = 1,
    ACK_MODE    = 2,

    // Direct Drop
    DROP_NOW    = 100,

    // Indirect Drop
    SIGNAL      = 200,
    ACK_SIGNAL  = 201,
    REVOKE      = 202,
    ACK_REVOKE  = 203,

    // Direct & Indirect
    ABOUT_TO_RELEASE = 255,
};

enum ad_mode {
    DIRECT_DROP   = 0,
    INDIRECT_DROP = 1,
};

// These currently map directly to their values in the protobuf enum,
// but they are redefined here so they are explicitly defined for the
// airdrop communication protocol.
enum ad_bottle {
    BOTTLE_A = 1,
    BOTTLE_B = 2,
    BOTTLE_C = 3,
    BOTTLE_D = 4,
    BOTTLE_E = 5,
};

// Validate that a packet is of header hdr and is valid
inline int validate_packet_as(enum ad_packet_hdr hdr, ad_packet_t packet) {
    if (packet.hdr != hdr) {
        return 0;
    }

    switch (packet.hdr) {
        case HEARTBEAT:
        case ACK_MODE:
            return 1;
        case SET_MODE:
            return (packet.data == DIRECT_DROP || packet.data == INDIRECT_DROP);
        case DROP_NOW:
        case SIGNAL:
        case ACK_SIGNAL:
        case REVOKE:
        case ACK_REVOKE:
        case ABOUT_TO_RELEASE:
            return (packet.data >= BOTTLE_A && packet.data <= BOTTLE_E);
        default:
            return 0;
    }
}

// Validate that any arbitrary packet is in the correct format
inline int validate_packet_any(ad_packet_t packet) {
    return validate_packet_as((enum ad_packet_hdr) packet.hdr, packet);
}

#endif  // INCLUDE_AIRDROP_PACKET_H_
