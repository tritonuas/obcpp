#ifndef INCLUDE_NETWORK_CLIENT_HPP_
#define INCLUDE_NETWORK_CLIENT_HPP_

#include <iostream>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include "rpi_connection.hpp"

namespace asio = boost::asio;

class Client {
    private:
        asio::ip::tcp::socket socket_;
        std::string ip;
        int port;

    public:

        // TODO: should probably make the io_context a unique_ptr
        Client(asio::io_context* io_context_, std::string ip, int port);

        // TODO: do we need a destructor? probably to free the socket right?
		// ~Client();

        /**
         * Connects to the specified ip and port
         */
        bool connect();

        /**
         * 
         */
        // TODO: so send is supposed to send a char but im unsure what the type should be uchar, uint8_t, etc.
        bool send(std::uint8_t request);

        /**
         * Reads in the header and fills a Header struct
         */
        // std::optional<Header>
        Header recvHeader();

        /**
         * Reads the actual data specified by the header
         */
        // std::optional<std::vector<std::uint8_t>>
        std::vector<std::uint8_t> recvBody(const int bufSize);

};

#endif
