#include <iostream>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include "camera_data.hpp"
#include "serialize.hpp"

#ifndef CLIENT_H
#define CLIENT_H

namespace asio = boost::asio;

class Client {
    private:
        asio::ip::tcp::socket socket_;
        std::string ip;
        int port;
        
        // underlying buffer for communication
        asio::streambuf sendbuf;
        asio::streambuf receivebuf;

        // TODO: these error codes are useless if we handle them on the spot of failure

        // free functions error codes
        boost::system::error_code ec_write;
        boost::system::error_code ec_read;

        // socket specific error codes
        boost::system::error_code ec_shutdown;
        boost::system::error_code ec_close;

        // image buffer
        std::vector<cv::Mat> images;

        /**
         * Creates the request from the request type
         */
        CameraRequest_t createRequest(RequestType_t requestType);

        /**
         * Creates a packet to send based off request type
         */
        void createRequestPacket(RequestType_t requestType);

        /**
         * Send the header of the packet
         */
        void sendHeader();

        /**
         * Send the body of the packet
         */
        void sendBody();

        /**
         * Read in the header of the packet
         * 
         * @return std::uint32_t Size of the body to be received
         */
        std::uint32_t receiveHeader();

        /** 
         * Read in the body of the packet
         * 
         * @param size Size of packet body
         */
        void receiveBody(std::uint32_t size);

        /**
         * Deconstructs the received packet
         */
        CameraResponse_t deconstructPacket();

        /**
         * Handler for what is received
         */
        void handlePacket();

    public:
        Client(asio::io_context* io_context_, std::string ip, int port);

        /**
         * Connects to the server with the specified ip and port
         */
        void connect();

        /**
         * Sends a packet to the server
         */
        void send(RequestType_t requestType);

        /**
         * Reads in packets send from the server
         */
        void receive();

        /**
         * Closes the connection between client and server
         */
        void disconnect();

        /**
         * This is a testing function to see what images are in the image buffer
         */
        void showImages();
};

#endif
