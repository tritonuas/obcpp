#include <gtest/gtest.h>

#include <future>
#include <thread>
#include <atomic>

extern "C" {
    #include "network/mock/packet_queue.h"
    #include "udp_squared/protocol.h"
    #include "network/airdrop_sockets.h"
}

// Make sure that the queue starts Empty and Not Full
TEST(PacketQueueTest, InitEmpty) {
    packet_queue_t q;
    pqueue_init(&q);

    ASSERT_TRUE(pqueue_empty(&q));
    ASSERT_FALSE(pqueue_full(&q));
}

// Make sure that pushing and popping one item works correctly
TEST(PacketQueueTest, PushThenPopOne) {
    packet_queue_t q;
    pqueue_init(&q);

    packet_t in_packet = makeModePacket(SET_MODE, UDP2_C, OBC_NULL, GUIDED);
    pqueue_push(&q, in_packet);
    ASSERT_FALSE(pqueue_empty(&q));
    ASSERT_FALSE(pqueue_full(&q));

    packet_t out_packet = pqueue_pop(&q);
    ASSERT_TRUE(pqueue_empty(&q));
    ASSERT_FALSE(pqueue_full(&q));
    ASSERT_EQ(in_packet.header, out_packet.header);
    ASSERT_EQ(in_packet.id, out_packet.id);
    mode_packet_t* out_mode_p = reinterpret_cast<mode_packet_t*>(&out_packet);
    ASSERT_EQ(out_mode_p->mode, GUIDED);
    ASSERT_EQ(out_mode_p->version, UDP2_VERSION);
}

// Make sure that pushing and popping many times works
TEST(PacketQueueTest, PushThenPopMany) {
    packet_queue_t q;
    pqueue_init(&q);

    for (int i = 0; i < MAX_PACKETS * 2; i++) {
        packet_t in_packet = makeResetPacket(UDP2_A);
        pqueue_push(&q, in_packet);
        ASSERT_FALSE(pqueue_empty(&q));
        ASSERT_FALSE(pqueue_full(&q));

        packet_t out_packet = pqueue_pop(&q);
        ASSERT_TRUE(pqueue_empty(&q));
        ASSERT_FALSE(pqueue_full(&q));
        ASSERT_EQ(in_packet.header, out_packet.header);
        ASSERT_EQ(in_packet.id, out_packet.id);
    }
}

// Make sure that pushing until capacity works, and that we can
// pop everything afterwards
TEST(PacketQueueTest, PushUntilFullThenPopUntilEmpty) {
    packet_queue_t q;
    pqueue_init(&q);

    for (int i = 0; i < MAX_PACKETS; i++) {
        packet_t p = makeResetPacket(static_cast<bottle_t>(i));
        pqueue_push(&q, p);
        ASSERT_FALSE(pqueue_empty(&q));
    }

    ASSERT_TRUE(pqueue_full(&q));

    for (int j = 0; j < MAX_PACKETS; j++) {
        packet_t p = pqueue_pop(&q);
        ASSERT_EQ(p.header, RESET);
        ASSERT_FALSE(pqueue_full(&q));
    }

    ASSERT_TRUE(pqueue_empty(&q));
}

// Test to make sure a combination of pushes and pops works like you would ASSERT
TEST(PacketQueueTest, SimulateNormalUse) {
    packet_queue_t q;
    packet_t temp;
    pqueue_init(&q);
    ASSERT_TRUE(pqueue_empty(&q));

    const int BURST = 10;
    for (int i = 0; i < BURST; i++) {
        pqueue_push(&q, makeResetPacket(UDP2_D));
    }

    temp = pqueue_pop(&q);
    ASSERT_EQ(temp.header, RESET);
    uint8_t bottle, state;
    parseID(temp.id, &bottle, &state);
    ASSERT_EQ(bottle, UDP2_D);
    temp = pqueue_pop(&q);
    ASSERT_EQ(temp.header, RESET);
    parseID(temp.id, &bottle, &state);
    ASSERT_EQ(bottle, UDP2_D);

    for (int i = BURST; i < BURST * 2; i++) {
        pqueue_push(&q, makeResetPacket(UDP2_D));
    }

    temp = pqueue_pop(&q);
    ASSERT_EQ(temp.header, RESET);
    parseID(temp.id, &bottle, &state);
    ASSERT_EQ(bottle, UDP2_D);
    temp = pqueue_pop(&q);
    ASSERT_EQ(temp.header, RESET);
    parseID(temp.id, &bottle, &state);
    ASSERT_EQ(bottle, UDP2_D);

    for (int i = BURST * 2; i < BURST * 3; i++) {
        pqueue_push(&q, makeResetPacket(UDP2_D));
    }

    for (int i = 4; i < BURST * 3; i++) {
        temp = pqueue_pop(&q);
        ASSERT_EQ(temp.header, RESET);
        parseID(temp.id, &bottle, &state);
        ASSERT_EQ(bottle, UDP2_D);
    }

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

    auto future = std::async(std::launch::async, [&q, &go]() {
        while (!go); // wait for other thread to run first
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        pqueue_push(&q, makeResetPacket(UDP2_C));
    });

    go = true;
    // There is still a chance that a context switch will occur here and let the other
    // thread run before this actually gets a chance to execute. That is what the
    // sleep_for call in the async thread is trying to avoid. Hopefully,
    // the 100 milliseconds will be enough so that this thread always gets scheduled again.
    packet_t p = pqueue_wait_pop(&q);

    ASSERT_EQ(p.header, RESET);
    uint8_t bottle, state;
    parseID(p.id, &bottle, &state);
    ASSERT_EQ(bottle, UDP2_C);
}