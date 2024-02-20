
#include <chrono>
#include <string>

#include "core/obc.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"

#include "network/airdrop_packets.h"

int main(int argc, char* argv[]) {
    initLogging(argc, argv);

    auto send_result = make_send_ad_socket();
    if (send_result.is_err) {
        LOG_F(ERROR, "send %s: %s", send_result.data.err_hdr, get_ad_send_err_msg());
        return 1;
    }

    int send_socket = send_result.data.sock_fd;

    auto recv_result = make_recv_ad_socket();
    if (recv_result.is_err) {
        LOG_F(ERROR, "recv %s: %s", recv_result.data.err_hdr, get_ad_recv_err_msg());
        return 1;
    }

    LOG_F(INFO, "worked");
    return 0;

    // In future, load configs, perhaps command line parameters, and pass
    // into the obc object
    OBC obc(DEFAULT_GCS_PORT);
    obc.run();
}
