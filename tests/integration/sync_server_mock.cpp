#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include "include/network/sync_server.hpp"

namespace asio = boost::asio;

const std::string ip {"127.0.0.1"};
constexpr uint32_t port {5000};

int main() {

    asio::io_context io_context_;

    Server server(&io_context_, ip, port);

    for (;;) {

        server.connect();

        while (true) {

            server.receive();

            if (server.getReadError() == asio::error::eof) {
                // TODO: what else do we do? can we still use the current socket to accept connections?
                break;
            }

            // TODO: should probably replace with handlePacket which will send() based on what was received? idk
            server.send(ResponseType_t::SUCC);

        }

        std::cout << "Client closed the connection" << '\n';

        server.close();
    }

    server.shutdown();

    return EXIT_SUCCESS;
}
