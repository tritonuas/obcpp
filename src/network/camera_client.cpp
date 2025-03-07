#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "network/camera_data.hpp"
#include "network/camera_client.hpp"
namespace asio = boost::asio;

CameraClient::CameraClient(int port)
{
    this->port=port;
}

bool CameraClient::connect(asio::io_context *io_context, std::string ip, int port)
{
    // create an asio endpoint
    asio::ip::tcp::endpoint endpoint(asio::make_adress(ip), port);

    // create a socket
    this->socket(&io_context);

    // connect socket
    this->socket::connect(&endpoint);

    // TODO: error handling

    return true;
}

// thinking if we just gonna run the same buff size and img proportions?
bool CameraClient::send(std::vector<std::uint8_t> image)
{
    // TODO: after finishing working version, try to serialize and send whole data type
    asio::buffer::const_buffer image_buffer = asio::buffer(&image, sizeof(image));

    //TODO: PUT ERROR DATA TYPE VAR HERE

    asio::socket::write(socket, image_buffrer); // TODO: PUT ERROR HANDLER HERE

    return true;
}
