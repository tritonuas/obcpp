#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "network/camera_data.hpp"
// https://www.codeproject.com/Articles/1264257/Socket-Programming-in-Cplusplus-using-boost-asio-T
namespace asio = boost::asio;

class CameraClient {
    public:
        /*
         * Create the socket
         * Params:
         *   - io_context (boost::asio::io_context)  - what io context to use
         *   - port (int)  - which port to occupy
         */
        CameraClient(int port);
        ~CameraClient();

        /*
         * Connect to server (OBC)
         * Params:
         *   - IP (string) - regular ip
         *   - port (int)  - port
         */
        bool connect(asio::io_context io_context, std::string ip, int port);

        /*
         * Sends pics out
         * Params:
         *   - header (string) - format should be "image_dimensions, buffer_size"
         *   - image (vector)  - vector of bytes of image
         * It should also probably have a terminator
         */
        bool send(std::vector<std::uint8_t> image);

        // Recieves a command to take pics 
        CameraResponse_t read();

    private:

        // ig?
        string ip;
        int port;
        asio::tcp::socket socket;
};