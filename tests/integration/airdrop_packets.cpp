#include <thread>
#include <chrono>

extern "C" {
    #include "airdrop/packet.h"
    #include "network/airdrop_sockets.h"
}

#include "network/airdrop_client.hpp"
#include "utilities/logging.hpp"

using namespace std::chrono_literals;

int main(int argc, char** argv) {
    initLogging("/workspaces/obcpp/logs", true, argc, argv);

    ad_socket_result_t result = make_ad_socket(AD_OBC_PORT, AD_PAYLOAD_PORT);

    if (result.is_err) {
        std::cerr << "SOCKET ERROR: " << result.data.err << std::endl;
        std::exit(1);
    }

    AirdropClient airdrop(result.data.res);
    airdrop.send(ad_latlng_packet_t {.hdr=SEND_LATLNG, .bottle=BOTTLE_A, .lat=32.12345, .lng=76.98765});
    std::this_thread::sleep_for(5s);
    LOG_F(INFO, "Checking for ack latlng...");
    while (true) {
        auto packet = airdrop.receive(); 
        if (!packet) {
            break;
        }
        LOG_F(INFO, "test %d %d", static_cast<int>(packet->data), static_cast<int>(packet->hdr));
    }
    LOG_F(INFO, "Done with checking for ack latlng");
    airdrop.send(ad_packet_t {.hdr=SIGNAL, .data=BOTTLE_A});
    std::this_thread::sleep_for(1s);
    while (true) {
        auto packet = airdrop.receive(); 
        if (!packet) {
            break;
        }
        LOG_F(INFO, "test2 %d %d", static_cast<int>(packet->data), static_cast<int>(packet->hdr));
    }
    airdrop.send(ad_packet_t {.hdr=REVOKE, .data=BOTTLE_A});
    std::this_thread::sleep_for(1s);
    while (true) {
        auto packet = airdrop.receive(); 
        if (!packet) {
            break;
        }
        LOG_F(INFO, "test2 %d %d", static_cast<int>(packet->data), static_cast<int>(packet->hdr));
    }

    while (true) {
        std::cout << "stop\n";
        std::this_thread::sleep_for(1s);
    }
}