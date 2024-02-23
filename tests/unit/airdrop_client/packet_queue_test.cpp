#include <gtest/gtest.h>

extern "C" {
    #include "packet_queue.h"
    #include "airdrop/packet.h"
    #include "network/airdrop_sockets.h"
}

TEST(PacketQueueTest, InitEmpty) {
    packet_queue_t q;
    pqueue_init(&q);

    EXPECT_TRUE(pqueue_empty(&q));
}

TEST(PacketQueueTest, PushThenPopOne) {
    packet_queue_t q;
    pqueue_init(&q);

    ad_packet_t in_packet = make_ad_packet(SET_MODE, DIRECT_DROP);
    pqueue_push(&q, in_packet);
    EXPECT_FALSE(pqueue_empty(&q));

    ad_packet_t out_packet = pqueue_pop(&q);
    EXPECT_TRUE(pqueue_empty(&q));
    EXPECT_EQ(in_packet.hdr, out_packet.hdr);
    EXPECT_EQ(in_packet.data, out_packet.data);
}

TEST(PacketQueueTest, PushThenPopMany) {
    packet_queue_t q;
    pqueue_init(&q);

    for (int i = 0; i < MAX_PACKETS * 2; i++) {
        ad_packet_t in_packet = make_ad_packet(i, i);
        pqueue_push(&q, in_packet);
        EXPECT_FALSE(pqueue_empty(&q));

        ad_packet_t out_packet = pqueue_pop(&q);
        EXPECT_TRUE(pqueue_empty(&q));
        EXPECT_EQ(in_packet.hdr, out_packet.hdr);
        EXPECT_EQ(in_packet.data, out_packet.data);
    }
}

TEST(PacketQueueTest, PushUntilFullThenPopUntilEmpty) {
    packet_queue_t q;
    pqueue_init(&q);

    for (int i = 0; i < MAX_PACKETS; i++) {
        ad_packet_t p = make_ad_packet(i, i + 2);
        pqueue_push(&q, p);
        std::cout << pqueue_empty(&q) << std::endl;
        ASSERT_FALSE(pqueue_empty(&q));
    }

    for (int j = 0; j < MAX_PACKETS; j++) {
        ad_packet_t p = pqueue_pop(&q);
        ASSERT_EQ(p.hdr, j);
        ASSERT_EQ(p.data, j + 2);
    }

    ASSERT_TRUE(pqueue_empty(&q));
}

TEST(PacketQueueTest, SimulateNormalUse) {
    packet_queue_t q;
    ad_packet_t temp;
    pqueue_init(&q);

    const int BURST = 10;
    for (int i = 0; i < BURST; i++) {
        pqueue_push(&q, make_ad_packet(i, i));
    }

    temp = pqueue_pop(&q);
    ASSERT_EQ(temp.hdr, 0);
    ASSERT_EQ(temp.data, 0);
    temp = pqueue_pop(&q);
    ASSERT_EQ(temp.hdr, 1);
    ASSERT_EQ(temp.data, 1);

    for (int i = BURST; i < BURST * 2; i++) {
        pqueue_push(&q, make_ad_packet(i, i));
    }

    temp = pqueue_pop(&q);
    ASSERT_EQ(temp.hdr, 2);
    ASSERT_EQ(temp.data, 2);
    temp = pqueue_pop(&q);
    ASSERT_EQ(temp.hdr, 3);
    ASSERT_EQ(temp.data, 3);

    for (int i = BURST * 2; i < BURST * 3; i++) {
        pqueue_push(&q, make_ad_packet(i, i));
    }

    for (int i = 4; i < BURST * 3; i++) {
        temp = pqueue_pop(&q);
        ASSERT_EQ(temp.hdr, i);
        ASSERT_EQ(temp.data, i);
    }

    ASSERT_TRUE(pqueue_empty(&q));

    pqueue_push(&q, make_ad_packet(HEARTBEAT, 0));
    pqueue_push(&q, make_ad_packet(HEARTBEAT, 0));
    pqueue_push(&q, make_ad_packet(HEARTBEAT, 0));
    pqueue_push(&q, make_ad_packet(HEARTBEAT, 0));

    ASSERT_FALSE(pqueue_empty(&q));

    ASSERT_EQ(pqueue_pop(&q).hdr, HEARTBEAT);
    ASSERT_EQ(pqueue_pop(&q).hdr, HEARTBEAT);
    ASSERT_EQ(pqueue_pop(&q).hdr, HEARTBEAT);
    ASSERT_EQ(pqueue_pop(&q).hdr, HEARTBEAT);

    ASSERT_TRUE(pqueue_empty(&q));
}
