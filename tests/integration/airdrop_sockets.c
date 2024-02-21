#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "airdrop/packet.h"
#include "network/airdrop_sockets.h"

#define ASSERT_ERROR(result) \
    if (result.is_err) { \
        fprintf(stderr, "%s\n", sr.data.err); \
        exit(1); \
    }

/*
 * Use:
 *   1 ./airdrop_sockets client 
 *     Starts up a client which broadcasts a packet every second until the server acks it.
 *     Once the server acks the packet, the client will exit.
 * 
 *   2 ./airdrop_sockets server
 *     Starts up a server which will listen on the socket for any packet, then ack it. 
 */

const uint16_t SERVER_LISTEN_PORT = 50000;
const uint16_t CLIENT_LISTEN_PORT = 50001;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Incorrect usage. See airdrop_sockets.c for an explanation.\n");
        exit(1);
    }

    ad_socket_result_t sr;
    ad_int_result_t ir;
    const size_t BUF_LEN = sizeof(ad_packet_t);
    char buf[BUF_LEN];

    if (strcmp(argv[1], "client") == 0) {
        sr = make_ad_socket(CLIENT_LISTEN_PORT, SERVER_LISTEN_PORT);
        ASSERT_ERROR(sr);

        ad_socket_t socket = sr.data.res; // successfully made the socket

        ir = set_socket_nonblocking(socket.fd);
        ASSERT_ERROR(ir);
        // successfully set the socket into nonblocking mode

        // This test is doing things single threaded, so these are in the same thread.
        set_send_thread();
        set_recv_thread();

        while (1) {
            ad_packet_t packet = make_ad_packet(SET_MODE, INDIRECT_DROP);
            ir = send_ad_packet(socket, &packet);
            ASSERT_ERROR(sr);        
            printf("Client sent packet.\n");

            ir = recv_ad_packet(socket, (void*) &buf[0], BUF_LEN);
            ASSERT_ERROR(sr);

            if (ir.data.res == AD_RECV_NOPACKETS) {
                printf("Client received no packet....\n");
                sleep(1);
                continue;
            }

            ad_packet_t* new_packet = (ad_packet_t*) buf;
            printf("Client received packet: %hhu %hhu\n", new_packet->hdr, new_packet->data);
            exit(0);
        }
    } else if (strcmp(argv[1], "server") == 0) {
        sr = make_ad_socket(SERVER_LISTEN_PORT, CLIENT_LISTEN_PORT);
        ASSERT_ERROR(sr);

        ad_socket_t socket = sr.data.res; // successfully made the socket

        // This test is doing things single threaded, so these are in the same thread.
        set_send_thread();
        set_recv_thread();
        while (1) {
            // block until packet received
            printf("Server waiting for packet...\n");
            ir = recv_ad_packet(socket, (void*) &buf[0], BUF_LEN);
            ASSERT_ERROR(ir);

            ad_packet_t* packet = (ad_packet_t*) buf;
            printf("Server received packet: %hhu %hhu\n", packet->hdr, packet->data);

            ad_packet_t send_packet = make_ad_packet(ACK_MODE, packet->data);
            ir = send_ad_packet(socket, &send_packet);
            ASSERT_ERROR(ir);
            printf("Server sent ack packet.");
        }
    }

    fprintf(stderr, "Need to specify server or client in first command line argument.\n");
    exit(1);
}