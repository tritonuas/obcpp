#include <gtest/gtest.h>

#include "network/airdrop_client.hpp"
extern "C" {
    #include "network/airdrop_sockets.h"
}
#include "utilities/common.hpp"

// Helper macro to initialize an airdrop client correctly
#define INIT_CLIENT() \
    auto result1 = make_ad_socket(AD_OBC_PORT, AD_PAYLOAD_PORT); \
    ASSERT_FALSE(result1.is_err); \
    auto socket = result1.data.res; \
    AirdropClient client(socket); \
    ASSERT_FALSE(client.isConnectionEstablished())

#define INIT_PAYLOAD_SOCKET() \
    auto result2 = make_ad_socket(AD_PAYLOAD_PORT, AD_OBC_PORT); \
    ASSERT_FALSE(result2.is_err); \
    auto payload_socket = result2.data.res

// Test to make sure that the client is initialized to not be connected
TEST(AirdropClientTest, InitClient) {
    INIT_CLIENT();
}

// Test to make sure that the client can be connected when it receives
// a SET_MODE message from the payloads
TEST(AirdropClientTest, ConnectClient) {
    INIT_CLIENT();
    INIT_PAYLOAD_SOCKET();

    auto future = std::async(std::launch::async, &AirdropClient::establishConnection, &client);

    send_ad_packet(payload_socket, make_ad_packet(SET_MODE, DIRECT_DROP));

    future.get();

    ASSERT_TRUE(client.isConnectionEstablished());
    ASSERT_EQ(client.getMode(), DIRECT_DROP);
}

TEST(AirdropClientTest, HeartbeatConnectionTestConstantHeartbeat) {
    INIT_CLIENT();
    INIT_PAYLOAD_SOCKET();

    // Sending heartbeats at a faster interval than the threshold, 
    // so the client should never record that a connection is lost
    const auto HEARTBEAT_INTERVAL = std::chrono::milliseconds(50);
    const auto CONN_LOSS_THRESHOLD = std::chrono::milliseconds(500);

    // Setting connection up
    auto _ = std::async(std::launch::async, &AirdropClient::establishConnection, &client);
    send_ad_packet(payload_socket, make_ad_packet(SET_MODE, DIRECT_DROP));
    _.get();

    // Spawn up a thread to simulate sending heartbeats from payloads every 50ms
    std::atomic_bool send_heartbeats = true;
    _ = std::async(std::launch::async, [payload_socket, &send_heartbeats, HEARTBEAT_INTERVAL]() {
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
    INIT_CLIENT();
    INIT_PAYLOAD_SOCKET();

    // Setting connection up
    auto _ = std::async(std::launch::async, &AirdropClient::establishConnection, &client);
    send_ad_packet(payload_socket, make_ad_packet(SET_MODE, DIRECT_DROP));
    _.get();


    // never send a heartbeat, wait a second..
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto lost_connections = client.getLostConnections(std::chrono::milliseconds(500));
    ASSERT_EQ(lost_connections.size(), NUM_AIRDROP_BOTTLES);
    for (const auto& [bottle_index, time_since_last_heartbeat] : lost_connections) {
        ASSERT_GT(time_since_last_heartbeat, std::chrono::milliseconds(500));
    }
}
