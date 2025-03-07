#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "camera_data.hpp"

// https://www.codeproject.com/Articles/1264257/Socket-Programming-in-Cplusplus-using-boost-asio-T
using namespace boost::asio;
using ip::tcp;

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
    explicit ObcCameraServer(int port);
    ~ObcCameraServer();

    /*
     * Connect to client (camera)
     * Params:
     *   - IP (string) - regular ip
     *   - port (int)  - port
     */
    bool connect(std::string ip, int port);

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
    string ip;
    int port;
    std::pair<int, int> camera_res; // used for reconstruction
    asio::tcp::socket socket;
};
