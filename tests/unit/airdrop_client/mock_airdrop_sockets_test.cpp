#include <gtest/gtest.h>

extern "C" {
    #include "network/airdrop_sockets.h"
    #include "udp_squared/protocol.h"
}

#define SETUP_SOCKETS() \
    auto r = make_ad_socket(UDP2_OBC_PORT, UDP2_PAYLOAD_PORT); \
    ASSERT_FALSE(r.is_err); \
    auto obc_socket = r.data.res; \
    r = make_ad_socket(UDP2_PAYLOAD_PORT, UDP2_OBC_PORT); \
    ASSERT_FALSE(r.is_err); \
    auto payload_socket = r.data.res

TEST(MockAirdropSocketTest, TestSendMessageObcToPayload) {
    SETUP_SOCKETS();
    packet_t p = { 0 };

    send_ad_packet(obc_socket, makeModePacket(SET_MODE, UDP2_A, OBC_NULL, GUIDED));
    recv_ad_packet(payload_socket, &p, sizeof(packet_t));
    mode_packet_t* mode_p = reinterpret_cast<mode_packet_t*>(&p);
    ASSERT_EQ(mode_p->header, SET_MODE);
    uint8_t airdrop, state;
    parseID(mode_p->id, &airdrop, &state);
    ASSERT_EQ(airdrop, UDP2_A);
    ASSERT_EQ(state, OBC_NULL);
    ASSERT_EQ(mode_p->mode, GUIDED);
    ASSERT_EQ(mode_p->version, UDP2_VERSION);

    p = { 0 };

    send_ad_packet(obc_socket, makeHeartbeatPacket(UDP2_A, OBC_NULL, 32.123, 76.321, 18, 90));
    recv_ad_packet(payload_socket, &p, sizeof(packet_t));
    heartbeat_packet_t* hb_p = reinterpret_cast<heartbeat_packet_t*>(&p);
    ASSERT_EQ(hb_p->header, HEARTBEAT);
    parseID(hb_p->id, &airdrop, &state);
    ASSERT_EQ(airdrop, UDP2_A);
    ASSERT_EQ(state,  OBC_NULL);
    ASSERT_FLOAT_EQ(hb_p->payload_lat, 32.123);
    ASSERT_FLOAT_EQ(hb_p->payload_lng, 76.321);
    ASSERT_EQ(hb_p->volts, 18);
    ASSERT_EQ(hb_p->altitude_m, 90);

    send_ad_packet(obc_socket, makeHeartbeatPacket(UDP2_A, OBC_NULL, 0.1, 7.1, 5, 91));
    send_ad_packet(obc_socket, makeHeartbeatPacket(UDP2_B, OBC_NULL, 0.2, 7.2, 6, 92));
    send_ad_packet(obc_socket, makeHeartbeatPacket(UDP2_C, OBC_NULL, 0.3, 7.3, 7, 93));
    send_ad_packet(obc_socket, makeHeartbeatPacket(UDP2_D, OBC_NULL, 0.4, 7.4, 8, 94));

    recv_ad_packet(payload_socket, &p, sizeof(packet_t));
    hb_p = reinterpret_cast<heartbeat_packet_t*>(&p);
    ASSERT_EQ(hb_p->header, HEARTBEAT);
    parseID(hb_p->id, &airdrop, &state);
    ASSERT_EQ(airdrop, UDP2_A);
    ASSERT_EQ(state,  OBC_NULL);
    ASSERT_FLOAT_EQ(hb_p->payload_lat, 0.1);
    ASSERT_FLOAT_EQ(hb_p->payload_lng, 7.1);
    ASSERT_EQ(hb_p->volts, 5);
    ASSERT_EQ(hb_p->altitude_m, 91);

    recv_ad_packet(payload_socket, &p, sizeof(packet_t));
    hb_p = reinterpret_cast<heartbeat_packet_t*>(&p);
    ASSERT_EQ(hb_p->header, HEARTBEAT);
    parseID(hb_p->id, &airdrop, &state);
    ASSERT_EQ(airdrop, UDP2_B);
    ASSERT_EQ(state,  OBC_NULL);
    ASSERT_FLOAT_EQ(hb_p->payload_lat, 0.2);
    ASSERT_FLOAT_EQ(hb_p->payload_lng, 7.2);
    ASSERT_EQ(hb_p->volts, 6);
    ASSERT_EQ(hb_p->altitude_m, 92);

    recv_ad_packet(payload_socket, &p, sizeof(packet_t));
    hb_p = reinterpret_cast<heartbeat_packet_t*>(&p);
    ASSERT_EQ(hb_p->header, HEARTBEAT);
    parseID(hb_p->id, &airdrop, &state);
    ASSERT_EQ(airdrop, UDP2_C);
    ASSERT_EQ(state,  OBC_NULL);
    ASSERT_FLOAT_EQ(hb_p->payload_lat, 0.3);
    ASSERT_FLOAT_EQ(hb_p->payload_lng, 7.3);
    ASSERT_EQ(hb_p->volts, 7);
    ASSERT_EQ(hb_p->altitude_m, 93);

    recv_ad_packet(payload_socket, &p, sizeof(packet_t));
    hb_p = reinterpret_cast<heartbeat_packet_t*>(&p);
    ASSERT_EQ(hb_p->header, HEARTBEAT);
    parseID(hb_p->id, &airdrop, &state);
    ASSERT_EQ(airdrop, UDP2_D);
    ASSERT_EQ(state,  OBC_NULL);
    ASSERT_FLOAT_EQ(hb_p->payload_lat, 0.4);
    ASSERT_FLOAT_EQ(hb_p->payload_lng, 7.4);
    ASSERT_EQ(hb_p->volts, 8);
    ASSERT_EQ(hb_p->altitude_m, 94);

    recv_ad_packet(payload_socket, &p, sizeof(packet_t));
    hb_p = reinterpret_cast<heartbeat_packet_t*>(&p);
    ASSERT_EQ(hb_p->header, HEARTBEAT);
    parseID(hb_p->id, &airdrop, &state);
    ASSERT_EQ(airdrop, UDP2_D);
    ASSERT_EQ(state,  OBC_NULL);
    ASSERT_FLOAT_EQ(hb_p->payload_lat, 0.5);
    ASSERT_FLOAT_EQ(hb_p->payload_lng, 7.5);
    ASSERT_EQ(hb_p->volts, 9);
    ASSERT_EQ(hb_p->altitude_m, 95);
}