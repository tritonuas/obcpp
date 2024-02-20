#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>

/**********************************************************************************************
 * This file defines the packet definitions for Airdrop communications. It is used in two
 * separate repositories: 
 * 
 * 1. https://github.com/tritonuas/guided-payload
 * 2. https://github.com/tritonuas/obcpp
 * 
 * This file was purposefully written to work in both C and C++. This is to ensure that whatever
 * hardware the payloads themselves run is able to include this code.
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
 * 
 */

const uint16_t AD_BROADCAST_PORT = 45906;

const struct sockaddr_in BROADCAST_ADDR {
    .sin_family = AF_INET,
    .sin_port = htons(AD_BROADCAST_PORT),
    .sin_addr = INADDR_ANY,
    .sin_zero = {0},
};

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

struct __attribute__ ((packed)) ad_packet {
    uint8_t hdr;
    uint8_t data;
};
typedef struct ad_packet ad_packet_t;

// For use in retrieving error messages
// Initialized in correspdoning make_socket function
static locale_t _send_locale = (locale_t) 0;
static locale_t _recv_locale = (locale_t) 0;

// Uses malloc to manually allocate an airdrop packet.
// DO NOT MANUALLLY USE delete TO FREE THIS MEMORY
// Instead, it will be free'd when consumed by send_packet
ad_packet_t* make_ad_packet(uint8_t hdr, uint8_t data) {
    ad_packet_t* packet = (ad_packet_t*) malloc(sizeof(ad_packet_t));
    packet->hdr = hdr;
    packet->data = data;
    return packet;
}

// Get most recent error message for single threaded use,
// or for when creating the socket initially
const char* get_ad_err() {
    return strerror(errno);
}

// Get error message for previous error with a send socket
const char* get_ad_send_thread_err() {
    return strerror_l(errno, _send_locale);
}

// Get error message for previous error with a recv socket
const char* get_ad_recv_thread_err() {
    return strerror_l(errno, _recv_locale);
}

// Need to call these two functions from corresponding thread
// if doing things multithreaded. This is so that the error messages
// between the two different threads won't override each other.
void set_send_thread() {
    // Tie error messages in this thread to the send locale
    _send_locale = newlocale(LC_ALL_MASK, "", (locale_t) 0);
    uselocale(_send_locale);
}
void set_recv_thread() {
    // Tie error messages in this thread to the recv locale
    _recv_locale = newlocale(LC_ALL_MASK, "", (locale_t) 0);
    uselocale(_recv_locale);
}

struct ad_socket_result {
    union {
        const char* err_hdr;  // where the error occurred
        int sock_fd;          // file descriptor (success)
    } data;
    uint8_t is_err;
};
typedef ad_socket_result ad_socket_result_t;

// needs to be as large as the longest str set in
// make_send_ad_socket or make_recv_ad_socket
#define AD_ERR_LEN 99

// Makes a socket for sending airdrop packets
// If the return value is negative, then an error occured
ad_socket_result make_ad_socket() {
    static char err[AD_ERR_LEN];
    int sock_fd;

    // Using IPv4 addrs with UDP datagrams
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        strncpy(&err[0], "socket failed", AD_ERR_LEN);
        return ad_socket_result_t {
            .data = { .err_hdr = &err[0] },
            .is_err = 1,
        };
    }

    // Allow broadcasting on the socket
    int broadcast_permission = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, 
                  (void*) &broadcast_permission, 
                  sizeof(broadcast_permission)) < 0) {
        strncpy(&err[0], "setsockopt broadcast failed", AD_ERR_LEN);
        return ad_socket_result_t {
            .data = { .err_hdr = &err[0] },
            .is_err = 1,
        };
    }

    // Bind the socket to the broadcast address
    if (bind(sock_fd, (struct sockaddr*) &BROADCAST_ADDR, sizeof(BROADCAST_ADDR)) < 0) {
        strncpy(&err[0], "bind socket failed", AD_ERR_LEN);
        return ad_socket_result_t {
            .data = { .err_hdr = &err[0] },
            .is_err = 1,
        };
    }

    return ad_socket_result_t {
        .data = { .sock_fd = sock_fd },
        .is_err = 0,
    };
}

// Actual Send/Receive helper functions

// send packet on the network through sock_fd and then
// frees the memory pointed to by packet
int send_ad_packet(int sock_fd, ad_packet_t* packet) {
    int res = send(sock_fd, (void*) packet, sizeof(packet), 0);
    free(packet);
    return res;
}

// receive packet and place into buf, which is of length buf_len
int recv_ad_packet(int sock_fd, void* buf, size_t buf_len) {
    return recv(sock_fd, buf, buf_len, 0);
}