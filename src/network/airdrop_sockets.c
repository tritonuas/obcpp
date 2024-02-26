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

ad_packet_t make_ad_packet(uint8_t hdr, uint8_t data) {
    ad_packet_t packet = {
        .hdr = hdr,
        .data = data,
    };
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


ad_socket_result_t make_ad_socket(uint16_t recv_port, uint16_t send_port) {
    // send socket is simple
    static char err[AD_ERR_LEN];
    int sock_fd;

    // Using IPv4 addrs with UDP datagrams
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        snprintf(&err[0], AD_ERR_LEN, "socket failed: %s", strerror(errno));
        AD_RETURN_ERR_RESULT(socket, err);
    }

    // Allow broadcasting on the socket
    int broadcast_permission = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST,
                  (void*) &broadcast_permission,
                  sizeof(broadcast_permission)) < 0) {
        snprintf(&err[0], AD_ERR_LEN, "setsockopt broadcast failed: %s", strerror(errno));
        AD_RETURN_ERR_RESULT(socket, err);
    }

    struct sockaddr_in RECV_ADDR = {
        .sin_family = AF_INET,
        .sin_port = htons(recv_port),
        .sin_addr = INADDR_ANY,
        .sin_zero = {0},
    };

    // Bind the socket to the broadcast address for receiving
    if (bind(sock_fd, (struct sockaddr*) &RECV_ADDR, sizeof(RECV_ADDR)) < 0) {
        snprintf(&err[0], AD_ERR_LEN, "bind socket failed: %s", strerror(errno));
        AD_RETURN_ERR_RESULT(socket, err);
    }

    ad_socket_t s = {
        .recv_port = recv_port,
        .send_port = send_port,
        .fd = sock_fd,
    };

    AD_RETURN_SUCC_RESULT(socket, s);
}

ad_int_result_t set_socket_nonblocking(int sock_fd) {
    static char err[AD_ERR_LEN];

    int ret;
    if ((ret = fcntl(sock_fd, F_SETFL, fcntl(sock_fd, F_GETFL) | O_NONBLOCK)) < 0) {
        snprintf(&err[0], AD_ERR_LEN, "set nonblocking failed: %s", strerror(errno));
        AD_RETURN_ERR_RESULT(int, err);
    }

    AD_RETURN_SUCC_RESULT(int, ret);
}

ad_int_result_t send_ad_packet(ad_socket_t socket, ad_packet_t packet) {
    struct sockaddr_in SEND_ADDR = {
        .sin_family = AF_INET,
        .sin_port = htons(socket.send_port),
        .sin_addr = INADDR_BROADCAST,
        .sin_zero = {0},
    };

    static char err[AD_ERR_LEN];
    int bytes_sent = sendto(socket.fd, (void*) &packet, sizeof(ad_packet_t), 0,
                            (struct sockaddr*) &SEND_ADDR, sizeof(SEND_ADDR));

    if (bytes_sent < 0) {
        snprintf(&err[0], AD_ERR_LEN, "send failed: %s", strerror_l(errno, _send_locale));
        AD_RETURN_ERR_RESULT(int, err);
    }

    AD_RETURN_SUCC_RESULT(int, bytes_sent);
}

ad_int_result_t recv_ad_packet(ad_socket_t socket, void* buf, size_t buf_len) {
    static char err[AD_ERR_LEN];
    struct sockaddr_in RECV_ADDR = {
        .sin_family = AF_INET,
        .sin_port = htons(socket.recv_port),
        .sin_addr = INADDR_ANY,
        .sin_zero = {0},
    };
    socklen_t RECV_ADDR_LEN = sizeof(RECV_ADDR);
    int bytes_read = recvfrom(socket.fd, buf, buf_len, 0,
                              (struct sockaddr*) &RECV_ADDR, &RECV_ADDR_LEN);

    if (bytes_read < 0) {
        // Need to detect if this is the case when a nonblocking socket didn't find a msg
        // because we shouldn't report that to the user as an error in the result.
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            AD_RETURN_SUCC_RESULT(int, AD_RECV_NOPACKETS);
        }

        snprintf(&err[0], AD_ERR_LEN, "recv failed: %s", strerror_l(errno, _recv_locale));
        AD_RETURN_ERR_RESULT(int, err);
    }

    AD_RETURN_SUCC_RESULT(int, bytes_read);
}

ad_int_result_t close_ad_socket(ad_socket_t socket) {
    static char err[AD_ERR_LEN];
    if (close(socket.fd) < 0) {
        snprintf(&err[0], AD_ERR_LEN, "close failed: %s", strerror(errno));

        AD_RETURN_ERR_RESULT(int, err);
    }

    AD_RETURN_SUCC_RESULT(int, 0);
}
