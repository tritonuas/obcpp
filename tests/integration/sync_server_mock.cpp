#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include "network/mock_server.hpp"

namespace asio = boost::asio;

const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 5000; 

int main() {

    asio::io_context io_context_;
    Server server(&io_context_, SERVER_IP, SERVER_PORT);

    //for (;;) {
        // start the server in a while loop 
        server.start();
    //}

    return EXIT_SUCCESS;
}
