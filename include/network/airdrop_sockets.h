#ifndef INCLUDE_NETWORK_AIRDROP_SOCKETS_H_
#define INCLUDE_NETWORK_AIRDROP_SOCKETS_H_

#include <stdlib.h>

#include "airdrop/packet.h"

struct ad_socket_result {
    union {
        const char* err;  // error message
        int res;          // result value
    } data;
    uint8_t is_err;
};
typedef struct ad_socket_result ad_socket_result_t;

// Uses malloc to manually allocate an airdrop packet.
// DO NOT MANUALLLY USE delete TO FREE THIS MEMORY!
// Instead, it will be free'd when consumed by send_ad_packet.
ad_packet_t* make_ad_packet(uint8_t hdr, uint8_t data);



// Makes a socket for sending airdrop packets
// If the return value is negative, then an error occured
ad_socket_result_t make_ad_socket();

// Need to call these two functions from corresponding thread
// that will be doing the sending or the receiving.
// If they are being done in the same thread then that is okay, but
// these still need to be called.
void set_send_thread();
void set_recv_thread();

// Either returns an error string or an int value corresponding to the
// return value of fcntl, which probably isn't important for mose uses
// because we mainly care about whether or not an error occurred.
ad_socket_result_t set_socket_nonblocking(int sock_fd);

// Actual Send/Receive helper functions

// Send packet on the network through sock_fd and then frees the packet.
// Either returns an error string or the number of bytes sent.
// IMPORTANT: must have previously called set_send_thread from curr thread.
ad_socket_result_t send_ad_packet(int sock_fd, ad_packet_t* packet);

// Receive packet and place into buf, which is of length buf_len.
// Either returns an error string or the number of bytes read.
// Caveat: if the socket is set in nonblocking mode, then number of bytes read will
// be -1, but is_err will be 0.
// IMPORTANT: must have previously called set_recv_thread from curr thread.
ad_socket_result_t recv_ad_packet(int sock_fd, void* buf, size_t buf_len);
#define AD_RECV_NOPACKETS -1

#endif  // INCLUDE_NETWORK_AIRDROP_SOCKETS_H_
