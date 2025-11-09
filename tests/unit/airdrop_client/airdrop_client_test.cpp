#include <gtest/gtest.h>

#include "network/airdrop_client.hpp"
extern "C" {
    #include "network/airdrop_sockets.h"
}
#include "utilities/common.hpp"
#include "core/obc.hpp"

// Helper macro to initialize an airdrop client <-> airdrop socket connection
#define SETUP_NETWORK(MODE, CLI_NAME, PAYLOAD_NAME) \
    auto result2 = make_ad_socket(UDP2_PAYLOAD_PORT, UDP2_OBC_PORT); \
    auto result1 = make_ad_socket(UDP2_OBC_PORT, UDP2_PAYLOAD_PORT); \
    ASSERT_FALSE(result2.is_err); \
    ASSERT_FALSE(result1.is_err); \
    auto PAYLOAD_NAME = result2.data.res; \
    send_ad_packet(PAYLOAD_NAME, makeModePacket(SET_MODE, UDP2_A, OBC_NULL, MODE)); \
    auto s = result1.data.res; \
    AirdropClient CLI_NAME = AirdropClient(s)

// Test to make sure that the client can be connected when it receives
// a SET_MODE message from the payloads
TEST(AirdropClientTest, ConnectClient) {
    SETUP_NETWORK(GUIDED, client, payload_socket);

    EXPECT_EQ(client.getMode(), GUIDED);
}

TEST(AirdropClientTest, HeartbeatConnectionNeverSend) {
    SETUP_NETWORK(GUIDED, client, _);

    // never send a heartbeat, wait a second..
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto lost_connections = client.getLostConnections(std::chrono::milliseconds(500));
    EXPECT_EQ(lost_connections.size(), NUM_AIRDROPS);
    for (const auto& [airdrop_index, time_since_last_heartbeat] : lost_connections) {
        EXPECT_GE(time_since_last_heartbeat, std::chrono::milliseconds(500));
    }
}

TEST(AirdropClientTest, ObcSendToPayload) {
    SETUP_NETWORK(GUIDED, client, payload_socket);

    int num_received = 0;
    const int NUM_TO_SEND = 10;

    auto _ = std::async(std::launch::async, [&client]() {
        int num_sent = 0;
        while (num_sent < NUM_TO_SEND) {
            client.send(makeArmPacket(ARM, UDP2_A, OBC_NULL, 100));
            num_sent++;
        }
    });

    packet_t p = { 0 };
    while (num_received < NUM_TO_SEND + 1) {
        recv_ad_packet(payload_socket, &p, sizeof(packet_t));
        if (num_received > 1) { // ignore first two because will be ack_mode
            ASSERT_EQ(p.header, ARM);
            uint8_t airdrop, state;
            parseID(p.id, &airdrop, &state);
            ASSERT_EQ(airdrop, UDP2_A);
            ASSERT_EQ(state, OBC_NULL);
        }
        num_received++;
    }
}
