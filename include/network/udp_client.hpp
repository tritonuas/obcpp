#ifndef INCLUDE_NETWORK_UDP_CLIENT_HPP_
#define INCLUDE_NETWORK_UDP_CLIENT_HPP_

#include <iostream>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include "rpi_connection.hpp"

namespace asio = boost::asio;

class UDPClient {
    private:
        asio::ip::udp::socket socket_;
        std::string ip;
        int port;

    public:
        UDPClient(asio::io_context* io_context_, std::string ip, int port);

        // there isnt really a notion of connect with connectionless udp sockets
        bool connect();

        bool send(std::uint8_t request);

        Header recvHeader();

        std::vector<std::uint8_t> recvBody(const int bufSize);
};

#endif
