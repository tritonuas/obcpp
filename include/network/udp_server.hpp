#ifndef INCLUDE_NETWORK_UDP_SERVER_HPP_
#define INCLUDE_NETWORK_UDP_SERVER_HPP_

#include <iostream>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include "network/rpi_connection.hpp"

namespace asio = boost::asio;

class UDPServer {
    private:
        std::string ip;
        int port;
        asio::ip::udp::socket socket_;

        cv::Mat createBGR();
        
        cv::Mat createYUV();

    public:
        UDPServer(asio::io_context* io_context_, std::string ip, int port);

        bool start();

        void send(asio::ip::udp::endpoint & endpoint);

        void recv();

        void handleRequest(char request, asio::ip::udp::endpoint & endpoint);

        void shutdown();
};

#endif  
