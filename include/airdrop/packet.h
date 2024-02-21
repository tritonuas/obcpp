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

#define AD_PAYLOAD_PORT 45906  // guided payloads listen on this port
#define AD_OBC_PORT     45907  // obc listens on this port

enum ad_packet_hdr {
    // Handshake to establish connection
    SET_MODE    = 0,
    ACK_MODE    = 1,

    // Direct Drop
    DROP_NOW    = 100,

    // Indirect Drop
    SIGNAL      = 200,
    ACK_SIGNAL  = 201,
    REVOKE      = 202,
    ACK_REVOKE  = 203,

    // Direct & Indirect
    DROPPED     = 999,
};

enum ad_mode {
    DIRECT_DROP   = 0,
    INDIRECT_DROP = 1,
};

enum ad_bottle {
    BOTTLE_A = 0,
    BOTTLE_B = 1,
    BOTTLE_C = 2,
    BOTTLE_D = 3,
    BOTTLE_E = 4,
};

// All of the values in here are 1 byte long, which means we do not have to worry about
// the endianness of the information as it is sent over the wire. If we ever add values more
// than one byte long, we'll have to worry about this.

// gcc specific packing syntax. Technically this should already be packed because it is just
// two one-byte values, but just making sure.
struct __attribute__ ((packed)) ad_packet {
    uint8_t hdr;
    uint8_t data;
};
typedef struct ad_packet ad_packet_t;

#endif  // INCLUDE_AIRDROP_PACKET_H_
