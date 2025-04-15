#include <opencv2/opencv.hpp>
#include "include/network/sync_server.hpp"


ImageData_t Server::createImageData() {

    ImageData_t i;
    cv::Mat img = cv::imread("birb.jpg"); // should be whatever the image is

    // converts a Mat to vector<uint_8> (Mat is of type CV_8UC3)
    cv::Mat flatten = img.reshape(1, img.total() * img.channels());
    std::vector<std::uint8_t> imgBuffer = img.isContinuous() ? flatten : flatten.clone();

    // fill struct with appropriate data
    i.height = img.rows;
    i.width = img.cols;
    i.imageSizeBytes = img.total() * img.elemSize();
    i.imgBuffer = imgBuffer;

    return i;

}

CameraResponse_t Server::createResponse(ResponseType_t responseType) {

    ImageData_t imgData = createImageData();

    // fill struct with appropriate data
    CameraResponse_t response;
    response.pid = 1010;
    response.responseType = responseType;
    response.imageData = imgData;

    return response;

}

void Server::createResponsePacket(ResponseType_t responseType) {

    CameraResponse_t response = createResponse(responseType);

    serialh::serialize(&response, &this->sendbuf);

}

void Server::sendHeader() {

    std::uint32_t headerLength = static_cast<std::uint32_t>(this->sendbuf.size());

    // TODO: error handler
    asio::write(this->socket_, asio::buffer(&headerLength, sizeof(headerLength)));

}

void Server::sendBody() {

    // TODO: error handler
    asio::write(this->socket_, this->sendbuf);

}

std::uint32_t Server::receiveHeader() {

    std::uint32_t size {};

    // TODO: error handler
    asio::read(this->socket_, asio::buffer(&size, sizeof(size)), ec_read);

    if (ec_read) {
        std::cout << "Failed to receive header: " << ec_read.message() << '\n';
    }

    return size;

}

void Server::receiveBody(std::uint32_t size) {

    // TODO: error handler
    asio::read(this->socket_, this->receivebuf.prepare(size));

    if (ec_read) {
        std::cout << "Failed to receive body: " << ec_read.message() << '\n';
    }

    this->receivebuf.commit(size);

}

CameraRequest_t Server::deconstructPacket() {

    CameraRequest_t request;

    serialh::deserialize(&request, &this->receivebuf);

    return request;

}

void Server::handlePacket() {
    
    // TODO: deconstruct packet
    CameraRequest_t request = deconstructPacket();

    std::cout << request.pid << '\n';
    std::cout << static_cast<int>(request.requestType) << '\n';

    // TODO: in this testing case prepare an image
    
}

// TODO: error handler (acceptor initialization)
Server::Server(asio::io_context* io_context_, std::string ip, int port) : socket_(*io_context_), acceptor_(*io_context_, asio::ip::tcp::endpoint(asio::ip::make_address(ip), port)) {

    this->ip = ip;
    this->port = port;

}

void Server::connect() {

    try {

        // TODO: error handler (switch to not exception?)
        this->acceptor_.accept(this->socket_);
        std::cout << "Listening on port: " << this->port << '\n';

    } catch (std::exception& e) {

        std::cout << e.what() << '\n';

    }

}

void Server::send(ResponseType_t responseType) {

    createResponsePacket(responseType);

    sendHeader();
    sendBody();

}

void Server::receive() {

    std::uint32_t bodySize = receiveHeader();
    receiveBody(bodySize);

    // TODO: if the client closes the connection, there won't be anything in the receivebuf and it throws an exception

    if (this->ec_read) {
        return;
    }

    handlePacket();
    
}

void Server::close() {

    // TODO: error handler
    this->socket_.shutdown(asio::ip::tcp::socket::shutdown_both);

    // TODO: error handler
    this->socket_.close();

}

void Server::shutdown() {

    this->acceptor_.close();
    close();
    this->socket_.release(); // TODO: unsure if this is correct

}

boost::system::error_code Server::getReadError() {

    return this->ec_read;

}

boost::system::error_code Server::getWriteError() {

    return this->ec_write;

}
