#include <iostream>
#include <boost/asio.hpp>
#include "camera_data.hpp"
#include "serialize.hpp"

namespace asio = boost::asio;

class Server {
    private:
        std::string ip;
        int port;
        asio::ip::tcp::socket socket_;
        asio::ip::tcp::acceptor acceptor_;

        asio::streambuf sendbuf;
        asio::streambuf receivebuf;

        // free functions
        boost::system::error_code ec_write;
        boost::system::error_code ec_read;

        // socket specific
        boost::system::error_code ec_shutdown;
        boost::system::error_code ec_close;

        // acceptor specific
        boost::system::error_code ec_acceptor_init;
        boost::system::error_code ec_accept;

        ImageData_t createImageData();
        /**
         * Creates the response from the request
         */
        CameraResponse_t createResponse(ResponseType_t response);

        /**
         * Creates a packet to send based off the request
         */
        void createResponsePacket(ResponseType_t response);

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
        CameraRequest_t deconstructPacket();

        /**
         * Handler for the packet based on what was received
         */
        void handlePacket();

    public:

        Server(asio::io_context* io_context_, std::string ip, int port);

        /**
         * Accepts connections on the underlying socket
         */
        void connect();

        /**
         * Sends a packet to the client
         */
        void send(ResponseType_t responseType);

        /**
         * Reads in packets send from the client
         */
        void receive();

        /**
         * Closes the socket
         */
        void close();

        /**
         * Shuts down the socket
         */
        void shutdown();
        
        /**
         * Returns the error if an error occurred on a read operation
         */
        boost::system::error_code getReadError();

        /**
         * Returns the error if an error occurred on a write operation
         */
        boost::system::error_code getWriteError();

};
