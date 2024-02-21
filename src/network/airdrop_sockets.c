// LINT_C_FILE

#include "network/airdrop_sockets.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "airdrop/packet.h"

// For use in retrieving error messages
// Initialized in correspdoning set_xxxx_thread function
static locale_t _send_locale = (locale_t) 0;
static locale_t _recv_locale = (locale_t) 0;

ad_packet_t* make_ad_packet(uint8_t hdr, uint8_t data) {
    ad_packet_t* packet = (ad_packet_t*) malloc(sizeof(ad_packet_t));
    packet->hdr = hdr;
    packet->data = data;
    return packet;
}

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

// Very large just in case strerror(errno) actually ends up being incredibly large
#define AD_ERR_LEN 999

#define AD_RETURN_ERR_RESULT(err) \
    ad_socket_result_t result = { \
        .data = { .err = &err[0] }, \
        .is_err = 1, \
    }; \
    return result

#define AD_RETURN_SUCC_RESULT(value) \
    ad_socket_result_t result = { \
        .data = { .res = value }, \
        .is_err = 0, \
    }; \
    return result

ad_socket_result_t make_ad_socket() {
    static char err[AD_ERR_LEN];
    int sock_fd;

    // Using IPv4 addrs with UDP datagrams
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        sprintf(&err[0], "socket failed: %s", strerror(errno));
        AD_RETURN_ERR_RESULT(err);
    }

    // Allow broadcasting on the socket
    int broadcast_permission = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST,
                  (void*) &broadcast_permission,
                  sizeof(broadcast_permission)) < 0) {
        sprintf(&err[0], "setsockopt broadcast failed: %s", strerror(errno));
        AD_RETURN_ERR_RESULT(err);
    }

    struct sockaddr_in RECV_ADDR = {
        .sin_family = AF_INET,
        .sin_port = htons(AD_OBC_PORT), // listen on the obc port
        .sin_addr = INADDR_ANY,
        .sin_zero = {0},
    };

    // Bind the socket to the broadcast address for receiving
    if (bind(sock_fd, (struct sockaddr*) &RECV_ADDR, sizeof(RECV_ADDR)) < 0) {
        sprintf(&err[0], "bind socket failed: %s", strerror(errno));
        AD_RETURN_ERR_RESULT(err);
    }

    struct sockaddr_in SEND_ADDR = {
        .sin_family = AF_INET,
        .sin_port = htons(AD_PAYLOAD_PORT), // send to the payload port
        .sin_addr = INADDR_ANY,
        .sin_zero = {0},
    };

    // Connect the socket to the broadcast address for sending
    if (connect(sock_fd, (struct sockaddr*) &SEND_ADDR, sizeof(SEND_ADDR)) < 0) {
        sprintf(&err[0], "connect socket failed: %s", strerror(errno));
        AD_RETURN_ERR_RESULT(err);
    }

    // disable loopback? TODO: figure out if this is needed
    int loopback = 0;
    if(setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (void *) &loopback, sizeof(loopback)) < 0) {
        sprintf(&err[0], "disable loopback failed: %s", strerror(errno));
        AD_RETURN_ERR_RESULT(err);
    }

    AD_RETURN_SUCC_RESULT(sock_fd);
}

ad_socket_result_t set_socket_nonblocking(int sock_fd) {
    static char err[AD_ERR_LEN];

    int ret;
    if((ret = fcntl(sock_fd, F_SETFL, fcntl(sock_fd, F_GETFL) | O_NONBLOCK)) < 0) {
        sprintf(&err[0], "set nonblocking failed: %s", strerror(errno));
        AD_RETURN_ERR_RESULT(err);
    }

    AD_RETURN_SUCC_RESULT(ret);
} 

ad_socket_result_t send_ad_packet(int sock_fd, ad_packet_t* packet) {
    static char err[AD_ERR_LEN];
    int bytes_sent = send(sock_fd, (void*) packet, sizeof(packet), 0);
    free(packet);

    if (bytes_sent < 0) {
        sprintf(&err[0], "send failed: %s", strerror_l(errno, _send_locale));
        AD_RETURN_ERR_RESULT(err);
    }

    AD_RETURN_SUCC_RESULT(bytes_sent);
}

ad_socket_result_t recv_ad_packet(int sock_fd, void* buf, size_t buf_len) {
    static char err[AD_ERR_LEN];
    int bytes_read = recv(sock_fd, buf, buf_len, 0);

    if (bytes_read < 0) {
        // Need to detect if this is the case when a nonblocking socket didn't find a msg
        // because we shouldn't report that to the user as an error in the result.
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            AD_RETURN_SUCC_RESULT(AD_RECV_NOPACKETS);
        }
        
        sprintf(&err[0], "recv failed: %s", strerror_l(errno, _recv_locale));
        AD_RETURN_ERR_RESULT(err);
    }

    AD_RETURN_SUCC_RESULT(bytes_read);
}
