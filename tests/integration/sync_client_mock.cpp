#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include "include/network/sync_client.hpp"

namespace asio = boost::asio;

const std::string ip {"127.0.0.1"};
constexpr uint32_t port {5000};

/*
what do we do when we are unable to ()?
what do we do when we fail to accept incoming connections?
what do we do when our send() fails?
what do we do when our write() fails?
what do we do when our shutdown fails?
what do we do when our close fails?
*/

/**
 * Method used for testing purposes, converts enum to string
 */
std::string convertEnumToString(RequestType_t request) {
    if (request == RequestType_t::SENDIMAGE) {
        return "SENDIMAGE";
    } else if (request == RequestType_t::PING) {
        return "PING";
    } else {
        return "SHUTDOWN";
    }
}

int main() {

    std::vector<RequestType_t> requests;
    requests.push_back(RequestType_t::SENDIMAGE);
    requests.push_back(RequestType_t::SENDIMAGE);
    requests.push_back(RequestType_t::SENDIMAGE);
    requests.push_back(RequestType_t::PING);
    requests.push_back(RequestType_t::SENDIMAGE);
    requests.push_back(RequestType_t::SENDIMAGE);
    requests.push_back(RequestType_t::SHUTDOWN);

    asio::io_context io_context_; 
    asio::streambuf buf;

    // Initialize the client
    Client client(&io_context_, ip, port);
    
    // TODO: what happens when unable to connect?
    client.connect();

    // Start sending messages
    for (RequestType_t request : requests) {

        std::cout << "Sending: " << convertEnumToString(request) << '\n';
        
        // send the buffer
        client.send(request);

        // TODO: should probably replace with handlePacket which will send() based on what was received? idk
        client.receive();
    }

    client.showImages();

    client.disconnect();

    return EXIT_SUCCESS;
}
