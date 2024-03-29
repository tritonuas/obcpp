#include <gtest/gtest.h>

#include "network/airdrop_client.hpp"
extern "C" {
    #include "network/airdrop_sockets.h"
}
#include "utilities/common.hpp"
#include "core/obc.hpp"

// Helper macro to initialize an airdrop client <-> airdrop socket connection
#define SETUP_NETWORK(MODE, CLI_NAME, PAYLOAD_NAME) \
    auto result2 = make_ad_socket(AD_PAYLOAD_PORT, AD_OBC_PORT); \
    auto result1 = make_ad_socket(AD_OBC_PORT, AD_PAYLOAD_PORT); \
    ASSERT_FALSE(result2.is_err); \
    ASSERT_FALSE(result1.is_err); \
    auto PAYLOAD_NAME = result2.data.res; \
    send_ad_packet(PAYLOAD_NAME, make_ad_packet(SET_MODE, MODE)); \
    auto s = result1.data.res; \
    AirdropClient CLI_NAME = AirdropClient(s);

// Test to make sure that the client can be connected when it receives
// a SET_MODE message from the payloads
TEST(AirdropClientTest, ConnectClient) {
    SETUP_NETWORK(DIRECT_DROP, client, payload_socket);

    ASSERT_EQ(client.getMode(), DIRECT_DROP);
}

TEST(AirdropClientTest, HeartbeatConnectionTestConstantHeartbeat) {
    SETUP_NETWORK(DIRECT_DROP, client, payload_socket);

    // Sending heartbeats at a faster interval than the threshold, 
    // so the client should never record that a connection is lost
    const auto HEARTBEAT_INTERVAL = std::chrono::milliseconds(50);
    const auto CONN_LOSS_THRESHOLD = std::chrono::milliseconds(500);

    // Spawn up a thread to simulate sending heartbeats from payloads every 50ms
    std::atomic_bool send_heartbeats = true;
    auto _ = std::async(std::launch::async, [payload_socket, &send_heartbeats, HEARTBEAT_INTERVAL]() {
        int times = 0; // bound this so it doesn't go forever in case of bug...
        while (send_heartbeats && times < 150) {
            for (int bottle = BOTTLE_A; bottle <= BOTTLE_E; bottle++) {
                send_ad_packet(payload_socket, make_ad_packet(HEARTBEAT, bottle));
            }
            times++;
            std::this_thread::sleep_for(HEARTBEAT_INTERVAL);
        }
    });
    
    const auto START_TIME = getUnixTime_ms();
    // Run for > 2s, make sure no connections are ever reported as lost
    while (getUnixTime_ms() - START_TIME <= std::chrono::milliseconds(1000)) {
        ASSERT_TRUE(client.getLostConnections(CONN_LOSS_THRESHOLD).empty());
    }

    send_heartbeats = false;
    _.get();
}

TEST(AirdropClientTest, HeartbeatConnectionNeverSend) {
    SETUP_NETWORK(DIRECT_DROP, client, _);

    // never send a heartbeat, wait a second..
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto lost_connections = client.getLostConnections(std::chrono::milliseconds(500));
    ASSERT_EQ(lost_connections.size(), NUM_AIRDROP_BOTTLES);
    for (const auto& [bottle_index, time_since_last_heartbeat] : lost_connections) {
        ASSERT_GE(time_since_last_heartbeat, std::chrono::milliseconds(500));
    }
}

TEST(AirdropClientTest, ObcSendToPayload) {
    SETUP_NETWORK(DIRECT_DROP, client, payload_socket);

    int num_received = 0;
    const int NUM_TO_SEND = 10;

    auto _ = std::async(std::launch::async, [&client]() {
        int num_sent = 0;
        while (num_sent < NUM_TO_SEND) {
            client.send(make_ad_packet(SIGNAL, num_sent));
            num_sent++;
        }
    });

    ad_packet_t p = { 0 };
    while (num_received < NUM_TO_SEND) {
        recv_ad_packet(payload_socket, &p, sizeof(ad_packet_t));
        ASSERT_EQ(p.hdr, SIGNAL);
        ASSERT_EQ(p.data, num_received);
        num_received++;
    }
}
