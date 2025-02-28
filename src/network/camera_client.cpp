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

CameraClient::connect(asio::io_context * io_context, std::string ip, int port)
{
    // create an asio endpoint
    asio::ip::tcp::endpoint endpoint(asio::make_adress(ip), port);

    // create a socket
    this->socket(&io_context);

    // connect socket
    this->socket::connect(&endpoint);
}