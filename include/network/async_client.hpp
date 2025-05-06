#ifndef INCLUDE_NETWORK_ASYNC_CLIENT_HPP_
#define INCLUDE_NETWORK_ASYNC_CLIENT_HPP_

#include <iostream>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include "camera_data.hpp"

namespace asio = boost::asio;

class Client {
    private:
        std::string ip;
        int port;
        // asio::ip::tcp::resolver resolver_;
        asio::ip::tcp::socket socket_; // TODO: does the socket need a lock?

        // TODO: likely need locks around these
        // TODO: scratch that, don't make shared class bufs, have the ownership be part of the async functions
        asio::streambuf sendbuf;
        asio::streambuf recvbuf;

        // TODO: this vector should really be protected by locks, what is the difference between the Locks provided in utilities/locks.hpp, utilities/lockptr.hpp, shared_mutex and any other mutex like objects i should be aware of
        std::queue<cv::Mat> images;

        // These functions will handle sending the request

        /**
         * This will take in the following request, make a "packet", serialize the data and then put it into the send buffer to send to the camera
         */
        void createRequestPacket(RequestType_t request);

        /**
         * 
         */
        void send(RequestType_t requestType);

        // These functions will handle reading the response

        /**
         * 
         */
        void deconstructPacket();

        /**
         * 
         */
        void recv();

        cv::Mat imgConvert();

    public:

        Client(asio::io_context* io_context_, std::string ip, int port);

        void connect();

        // TODO: unsure what this should return
        void ping();

        void takePicture();

        // TODO: return type should probably change
        cv::Mat getLatestImage();
        
};

#endif
