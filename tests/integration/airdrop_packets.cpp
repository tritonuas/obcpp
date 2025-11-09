#include <thread>
#include <chrono>

extern "C" {
    #include "udp_squared/protocol.h"
    #include "network/airdrop_sockets.h"
}

#include "network/airdrop_client.hpp"
#include "utilities/logging.hpp"

using namespace std::chrono_literals;

// TODO - figure out how to test this
//          - may need to actually connect with a physical airdrop?
int main(int argc, char** argv) {
    initLogging("/workspaces/obcpp/logs", true, argc, argv);

    ad_socket_result_t result = make_ad_socket(UDP2_OBC_PORT, UDP2_PAYLOAD_PORT);

    if (result.is_err) {
        std::cerr << "SOCKET ERROR: " << result.data.err << std::endl;
        std::exit(1);
    }

    AirdropClient airdrop(result.data.res);

    std::this_thread::sleep_for(10s);

    for (int i = 0; i < 16; i++) {
        std::cout << "DROP A\n";
        airdrop.send(makeDropNowPacket(airdrop_t::UDP2_A));
        std::this_thread::sleep_for(100ms);
    }
    for (int i = 0; i < 16; i++) {
        std::cout << "DROP B\n";
        airdrop.send(makeDropNowPacket(airdrop_t::UDP2_B));
        std::this_thread::sleep_for(100ms);
    }

    airdrop.send(makeLatLngPacket(SEND_LATLNG, UDP2_A, OBC_NULL, 32.123, 76.321, 100));
    airdrop.send(makeLatLngPacket(SEND_LATLNG, UDP2_A, OBC_NULL, 32.123, 76.321, 100));
    airdrop.send(makeLatLngPacket(SEND_LATLNG, UDP2_A, OBC_NULL, 32.123, 76.321, 100));
    airdrop.send(makeLatLngPacket(SEND_LATLNG, UDP2_A, OBC_NULL, 32.123, 76.321, 100));
    std::this_thread::sleep_for(3s);
    LOG_F(INFO, "Checking for ack latlng...");
    while (true) {
        auto packet = airdrop.receive(); 
        if (!packet) {
            break;
        }
        uint8_t airdrop, state;
        parseID(packet->id, &airdrop, &state);
        LOG_F(INFO, "rec ack latlng? %d %d %d", static_cast<int>(packet->header), static_cast<int>(airdrop), static_cast<int>(state));
    }
    LOG_F(INFO, "Done with checking for ack latlng");
    airdrop.send(makeArmPacket(ARM, UDP2_A, OBC_NULL, 105));
    airdrop.send(makeArmPacket(ARM, UDP2_A, OBC_NULL, 105));
    airdrop.send(makeArmPacket(ARM, UDP2_A, OBC_NULL, 105));
    airdrop.send(makeArmPacket(ARM, UDP2_A, OBC_NULL, 105));
    std::this_thread::sleep_for(3s);
    while (true) {
        auto packet = airdrop.receive(); 
        if (!packet) {
            break;
        }
        uint8_t airdrop, state;
        parseID(packet->id, &airdrop, &state);
        LOG_F(INFO, "recv ack arm? %d %d %d", static_cast<int>(packet->header), static_cast<int>(airdrop), static_cast<int>(state));
    }
    airdrop.send(makeArmPacket(DISARM, UDP2_A, OBC_NULL, 100));
    airdrop.send(makeArmPacket(DISARM, UDP2_A, OBC_NULL, 100));
    airdrop.send(makeArmPacket(DISARM, UDP2_A, OBC_NULL, 100));
    airdrop.send(makeArmPacket(DISARM, UDP2_A, OBC_NULL, 100));
    std::this_thread::sleep_for(3s);
    while (true) {
        auto packet = airdrop.receive(); 
        if (!packet) {
            break;
        }
        uint8_t airdrop, state;
        parseID(packet->id, &airdrop, &state);
        LOG_F(INFO, "recv ack disarm? %d %d %d", static_cast<int>(packet->header), static_cast<int>(airdrop), static_cast<int>(state));
    }

    while (true) {
        std::cout << "stop\n";
        std::this_thread::sleep_for(1s);
    }
}