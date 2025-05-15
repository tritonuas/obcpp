#ifndef INCLUDE_NETWORK_TCP_SERVER_HPP_
#define INCLUDE_NETWORK_TCP_SERVER_HPP_

#include <iostream>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>

namespace asio = boost::asio;

class Server {
    private:
        std::string ip;
        int port;
        asio::ip::tcp::socket socket_;
        asio::ip::tcp::acceptor acceptor_;

        void takePicture();

        cv::Mat createBGR();
        
        cv::Mat createYUV();

    public:
        Server(asio::io_context* io_context_, std::string ip, int port);

        void start();

        void send();

        void recv();

        void handleRequest(char request);

        void shutdown();
};

#endif 
