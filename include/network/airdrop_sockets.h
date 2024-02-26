#ifndef INCLUDE_NETWORK_AIRDROP_SOCKETS_H_
#define INCLUDE_NETWORK_AIRDROP_SOCKETS_H_

#include <stdlib.h>

#include "airdrop/packet.h"

struct ad_socket {
    uint16_t send_port;
    uint16_t recv_port;
    int fd;
};
typedef struct ad_socket ad_socket_t;

struct ad_socket_result {
    union {
        const char* err;
        ad_socket_t res;
    } data; \
    uint8_t is_err;
};
typedef struct ad_socket_result ad_socket_result_t;

struct ad_int_result {
    union {
        const char* err;
        int res;
    } data; \
    uint8_t is_err;
};
typedef struct ad_int_result ad_int_result_t;

#define AD_RETURN_ERR_RESULT(typename, err) \
    ad_##typename##_result_t result = { \
        .data = { .err = &err[0] }, \
        .is_err = 1, \
    }; \
    return result

#define AD_RETURN_SUCC_RESULT(typename, value) \
    ad_##typename##_result_t result = { \
        .data = { .res = value }, \
        .is_err = 0, \
    }; \
    return result

ad_packet_t make_ad_packet(uint8_t hdr, uint8_t data);

// TODO: write documentation
ad_socket_result_t make_ad_socket(uint16_t recv_port, uint16_t send_port);

// Need to call these two functions from corresponding thread
// that will be doing the sending or the receiving.
// If they are being done in the same thread then that is okay, but
// these still need to be called.
void set_send_thread();
void set_recv_thread();

// Either returns an error string or an int value corresponding to the
// return value of fcntl, which probably isn't important for mose uses
// because we mainly care about whether or not an error occurred.
ad_int_result_t set_socket_nonblocking(int sock_fd);

// Actual Send/Receive helper functions

// Send packet on the network through sock_fd
// Either returns an error string or the number of bytes sent.
// IMPORTANT: must have previously called set_send_thread from curr thread.
ad_int_result_t send_ad_packet(ad_socket_t socket, ad_packet_t packet);

// Receive packet and place into buf, which is of length buf_len.
// Either returns an error string or the number of bytes read.
// Caveat: if the socket is set in nonblocking mode, then number of bytes read will
// be -1, but is_err will be 0.
// IMPORTANT: must have previously called set_recv_thread from curr thread.
ad_int_result_t recv_ad_packet(ad_socket_t socket, void* buf, size_t buf_len);
#define AD_RECV_NOPACKETS -1

ad_int_result_t close_ad_socket(ad_socket_t socket);

#endif  // INCLUDE_NETWORK_AIRDROP_SOCKETS_H_
