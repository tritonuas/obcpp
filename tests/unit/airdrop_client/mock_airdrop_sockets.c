// LINT_C_FILE

/*
 * We want to have unit tests to test the connectivity of the airdrop client,
 * but they rely on socket system calls which depend on the operating system.
 * This is bad for unit tests because you don't want your unit tests to have
 * any dependencies outside the control of the program that could cause them
 * to randomly fail some percentage of the time. (In other other words, they
 * should be deterministic).
 * 
 * What this file does, then, is essentially provide a deterministic
 * implementation of the code in network/airdrop_sockets.c that does not
 * rely on any system calls nor any network request. Then, the unit test
 * can swap out the network/airdrop_sockets.c file for this one so that
 * it doesn't actually need anything from the OS or the network.
 */

#include "network/airdrop_sockets.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "airdrop/packet.h"
#include "packet_queue.h"

// Global variables to buffer the messages
static packet_queue_t obc_queue;
static packet_queue_t payload_queue;

ad_socket_result_t make_ad_socket(uint16_t recv_port, uint16_t send_port) {
    pqueue_init(&obc_queue);
    pqueue_init(&payload_queue);

    AD_RETURN_SUCC_RESULT(socket, 0);
}

ad_int_result_t send_ad_packet(ad_socket_t socket, ad_packet_t* packet) {
    static char err[1] = "";

    packet_queue_t* q;
    if (socket.send_port == AD_OBC_PORT) {
        q = &obc_queue;
    } else { // assume payload otherwise for the purposes of simple testing
        q = &obc_queue;
    }

    if (pqueue_full(q)) {
        AD_RETURN_ERR_RESULT(int, err);
    }

    ad_packet_t p = {
        .data = packet->data,
        .hdr = packet->hdr,
    };
    pqueue_push(q, p);

    AD_RETURN_SUCC_RESULT(int, sizeof(ad_packet_t));
}

ad_int_result_t recv_ad_packet(ad_socket_t socket, void* buf, size_t buf_len) {
    AD_RETURN_SUCC_RESULT(int, sizeof(ad_packet_t));
}

// Everything below here is not very interesting


ad_int_result_t set_socket_nonblocking(int sock_fd) {
    AD_RETURN_SUCC_RESULT(int, 0);
}

ad_int_result_t close_ad_socket(ad_socket_t socket) {
    AD_RETURN_SUCC_RESULT(int, 0);
}

ad_packet_t make_ad_packet(uint8_t hdr, uint8_t data) {
    ad_packet_t packet = {
        .hdr = hdr,
        .data = data,
    };
    return packet;
}

void set_send_thread() {}

void set_recv_thread() {}