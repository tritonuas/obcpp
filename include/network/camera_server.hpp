#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

// https://www.codeproject.com/Articles/1264257/Socket-Programming-in-Cplusplus-using-boost-asio-T
using namespace boost::asio;
using ip::tcp;


struct image_data_t {
    int width;
    int height;
    size_t image_size_bytes;
    std::vector<std::uint8_t> data;
};


class CameraServer
{
public:
    /*
     * Create the socket
     * Params:
     *   - port (int)  - which port to occupy
     */
    explicit CameraServer(int port);
    ~CameraServer();

    /*
     * Connect to server (OBC)
     * Params:
     *   - IP (string) - regular ip
     *   - port (int)  - port
     */
    bool connect(std::string ip, int port);

    /*
     * Sends command to take and return pic
     * Params:
     *   TODO: IDK WHAT TO SEND
     */
    bool send(std::string command);

    // Recieves a command to take pics
    image_data read();

private:
    // ig?
    string ip;
    int port;
    tcp::socket socket;
};
