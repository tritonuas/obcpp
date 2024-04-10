#include <gtest/gtest.h>

extern "C" {
    #include "network/airdrop_sockets.h"
}

#define SETUP_SOCKETS() \
    auto r = make_ad_socket(AD_OBC_PORT, AD_PAYLOAD_PORT); \
    ASSERT_FALSE(r.is_err); \
    auto obc_socket = r.data.res; \
    r = make_ad_socket(AD_PAYLOAD_PORT, AD_OBC_PORT); \
    ASSERT_FALSE(r.is_err); \
    auto payload_socket = r.data.res

TEST(MockAirdropSocketTest, TestSendMessageObcToPayload) {
    SETUP_SOCKETS();
    ad_packet_t p = { 0 };

    send_ad_packet(obc_socket, make_ad_packet(SET_MODE, DIRECT_DROP));
    recv_ad_packet(payload_socket, &p, sizeof(ad_packet_t));
    ASSERT_EQ(p.hdr, SET_MODE);
    ASSERT_EQ(p.data , DIRECT_DROP);

    send_ad_packet(obc_socket, make_ad_packet(HEARTBEAT, 77));
    recv_ad_packet(payload_socket, &p, sizeof(ad_packet_t));
    ASSERT_EQ(p.hdr, HEARTBEAT);
    ASSERT_EQ(p.data , 77);

    send_ad_packet(obc_socket, make_ad_packet(HEARTBEAT, 78));
    send_ad_packet(obc_socket, make_ad_packet(HEARTBEAT, 79));
    send_ad_packet(obc_socket, make_ad_packet(HEARTBEAT, 80));
    recv_ad_packet(payload_socket, &p, sizeof(ad_packet_t));
    ASSERT_EQ(p.hdr, HEARTBEAT);
    ASSERT_EQ(p.data , 78);
    recv_ad_packet(payload_socket, &p, sizeof(ad_packet_t));
    ASSERT_EQ(p.hdr, HEARTBEAT);
    ASSERT_EQ(p.data , 79);
    recv_ad_packet(payload_socket, &p, sizeof(ad_packet_t));
    ASSERT_EQ(p.hdr, HEARTBEAT);
    ASSERT_EQ(p.data , 80);
}