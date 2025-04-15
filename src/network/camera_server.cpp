#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "camera_data.hpp"
#include "camera_server.hpp"

// https://www.codeproject.com/Articles/1264257/Socket-Programming-in-Cplusplus-using-boost-asio-T
namespace asio = boost::asio;

// may throw an error due to acceptor construction
ObcCameraServer::ObcCameraServer(std::string ip, int port, asio::io_context& io_context_) : socket_(io_context_), acceptor_(asio::ip::tcp::endpoint(asio::ip::make_address(ip), port)) {

    this->ip = ip;
    this->port = port;

}

bool ObcCameraServer::connect() {

    try {

        // may throw an error
        this->acceptor_.accept(this->socket_);
        return true;

    } catch(std::exception& e) {

        return false;
    }

}

bool ObcCameraServer::send(RequestType_t type) {

    CameraRequest_t request;
    request.pid = 0; // TODO: no idea what pid is
    request.requestType = type;

    asio::streambuf buf;
    boost::system::error_code ec;

    // serialize the request struct
    // TODO: create helper functions (serialize.hpp)
    std::ostream os(&buf);
    boost::archive::binary_oarchive oa(os);
    oa << request;

    // send the serialized data, may result in an error
    asio::write(this->socket_, buf, ec);

    if (ec) {
        return false;
    }

    return true;
}

CameraResponse_t ObcCameraServer::read() {

    CameraResponse_t response;
    asio::streambuf buf;
    boost::system::error_code ec;

    // read the data from the socket, may result in an error
    asio::read(this->socket_, buf, ec);

    if (ec) {
        // TODO: what do we return if no data read
    }

    // deserialize the data
    // TODO: create helper functions (serialize.hpp)
    std::istream is(&buf);
    boost::archive::binary_iarchive ia(is);
    ia >> response;

    return response;
}


