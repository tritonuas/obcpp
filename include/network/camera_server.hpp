#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "camera_data.hpp"

// https://www.codeproject.com/Articles/1264257/Socket-Programming-in-Cplusplus-using-boost-asio-T
namespace asio = boost::asio;

// OBC-side socket.
// Sends requests to take pics to camera, sends it back up to 
class ObcCameraServer
{
public:
    /*
     * Create the socket
     * Params:
     *   - port (int)  - which port to occupy
     */
    explicit ObcCameraServer(std::string ip, int port, asio::io_context& io_context_);
    ~ObcCameraServer();

    /*
     * Connect to client (camera)
     * Params:
     *   - IP (string) - regular ip
     *   - port (int)  - port
     */
    bool connect();

    /*
     * Sends command to take and return pic
     * Params:
     *   - type (RequestType_t) - what OBC wants camera to do
     * Returns:
     *  - bool - if success or not
     */
    bool send(RequestType_t type);

    /*
     * Reads the response
     * Returns:
     *   - CameraResponse_t - response that camera passed in
     */
    CameraResponse_t read();

private:
    // ig?
    std::string ip;
    int port;
    std::pair<int, int> camera_res; // used for reconstruction
    asio::ip::tcp::socket socket_;
    asio::ip::tcp::acceptor acceptor_;
};
