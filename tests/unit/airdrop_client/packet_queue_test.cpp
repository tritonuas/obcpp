#include <gtest/gtest.h>

#include <future>
#include <thread>
#include <atomic>

extern "C" {
    #include "packet_queue.h"
    #include "airdrop/packet.h"
    #include "network/airdrop_sockets.h"
}

// Make sure that the queue starts Empty and Not Full
TEST(PacketQueueTest, InitEmpty) {
    packet_queue_t q;
    pqueue_init(&q);

    EXPECT_TRUE(pqueue_empty(&q));
    EXPECT_FALSE(pqueue_full(&q));
}

// Make sure that pushing and popping one item works correctly
TEST(PacketQueueTest, PushThenPopOne) {
    packet_queue_t q;
    pqueue_init(&q);

    ad_packet_t in_packet = make_ad_packet(SET_MODE, DIRECT_DROP);
    pqueue_push(&q, in_packet);
    EXPECT_FALSE(pqueue_empty(&q));
    EXPECT_FALSE(pqueue_full(&q));

    ad_packet_t out_packet = pqueue_pop(&q);
    EXPECT_TRUE(pqueue_empty(&q));
    EXPECT_FALSE(pqueue_full(&q));
    EXPECT_EQ(in_packet.hdr, out_packet.hdr);
    EXPECT_EQ(in_packet.data, out_packet.data);
}

// Make sure that pushing and popping many times works
TEST(PacketQueueTest, PushThenPopMany) {
    packet_queue_t q;
    pqueue_init(&q);

    for (int i = 0; i < MAX_PACKETS * 2; i++) {
        ad_packet_t in_packet = make_ad_packet(i, i);
        pqueue_push(&q, in_packet);
        EXPECT_FALSE(pqueue_empty(&q));
        EXPECT_FALSE(pqueue_full(&q));

        ad_packet_t out_packet = pqueue_pop(&q);
        EXPECT_TRUE(pqueue_empty(&q));
        EXPECT_FALSE(pqueue_full(&q));
        EXPECT_EQ(in_packet.hdr, out_packet.hdr);
        EXPECT_EQ(in_packet.data, out_packet.data);
    }
}

// Make sure that pushing until capacity works, and that we can
// pop everything afterwards
TEST(PacketQueueTest, PushUntilFullThenPopUntilEmpty) {
    packet_queue_t q;
    pqueue_init(&q);

    for (int i = 0; i < MAX_PACKETS; i++) {
        ad_packet_t p = make_ad_packet(i, i + 2);
        pqueue_push(&q, p);
        ASSERT_FALSE(pqueue_empty(&q));
    }

    EXPECT_TRUE(pqueue_full(&q));

    for (int j = 0; j < MAX_PACKETS; j++) {
        ad_packet_t p = pqueue_pop(&q);
        ASSERT_EQ(p.hdr, j);
        ASSERT_EQ(p.data, j + 2);
        EXPECT_FALSE(pqueue_full(&q));
    }

    ASSERT_TRUE(pqueue_empty(&q));
}

// Test to make sure a combination of pushes and pops works like you would expect
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

// To thoroughly test the multithreaded aspect of the queue we would definitely need a lot more
// tests. If any expert gamers would like to do so, then that would be epic I think.

// Test to make sure the async aspect of the queue works (i.e. pqueue_wait_pop correctly blocks)
// and gets unblocked by the semaphores.
TEST(PacketQueueTest, PopWaitMultiThreaded) {
    // this is just like fortnite
    packet_queue_t q;
    pqueue_init(&q);

    std::atomic_bool go = false;

    const ad_packet_hdr EXP_HDR = SIGNAL;
    const ad_bottle EXP_BTL = BOTTLE_C; 

    auto future = std::async(std::launch::async, [&q, &go]() {
        while (!go); // wait for other thread to run first
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        pqueue_push(&q, make_ad_packet(EXP_HDR, EXP_BTL));
    });

    go = true;
    // There is still a chance that a context switch will occur here and let the other
    // thread run before this actually gets a chance to execute. That is what the
    // sleep_for call in the async thread is trying to avoid. Hopefully,
    // the 100 milliseconds will be enough so that this thread always gets scheduled again.
    ad_packet_t p = pqueue_wait_pop(&q);

    ASSERT_EQ(p.hdr, EXP_HDR);
    ASSERT_EQ(p.data, EXP_BTL);
}
