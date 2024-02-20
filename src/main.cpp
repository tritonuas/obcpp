
#include <chrono>
#include <string>

#include "core/obc.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"

#include "network/airdrop_packets.h"

int main(int argc, char* argv[]) {
    initLogging(argc, argv);

    auto socket_result = make_ad_socket();
    if (socket_result.is_err) {
        LOG_F(ERROR, "%s: %s", socket_result.data.err_hdr, get_ad_err());
        return 1;
    }
    int socket = socket_result.data.sock_fd;

    send_ad_packet(socket, make_ad_packet(SET_MODE, DIRECT_DROP));

    char buf[100];
    recv_ad_packet(socket, &buf[0], 100);
    LOG_F(INFO, "%s", buf);
    return 0;

    // In future, load configs, perhaps command line parameters, and pass
    // into the obc object
    OBC obc(DEFAULT_GCS_PORT);
    obc.run();
}
