#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

// https://www.codeproject.com/Articles/1264257/Socket-Programming-in-Cplusplus-using-boost-asio-T
using namespace boost::asio;
using ip::tcp;

class CameraClient {
    public:
        /*
         * Create the socket
         * Params:
         *   - port (int)  - which port to occupy
         */
        explicit CameraClient(int port);
        ~CameraClient();

        /*
         * Connect to server (OBC)
         * Params:
         *   - IP (string) - regular ip
         *   - port (int)  - port
         */
        bool connect(std::string ip, int port);

        /*
         * Sends pics out
         * Params:
         *   - header (string) - format should be "image_dimensions, buffer_size"
         *   - image (vector)  - vector of bytes of image
         * It should also probably have a terminator
         */
        bool send(std::string header, std::vector<std::uint8_t> image);

        // Recieves a command to take pics 
        // TODO: figure out what should be return type
        bool read();

    private:
        // ig?
        string ip;
        int port;
        tcp::socket socket;
};