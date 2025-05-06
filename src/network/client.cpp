#include "network/client.hpp"

Client::Client(asio::io_context* io_context_, std::string ip, int port) : socket_(*io_context_) {

    this->ip = ip;
    this->port = port;

}

bool Client::connect() {
    boost::system::error_code ec;
    asio::ip::tcp::endpoint endpoint_(asio::ip::tcp::endpoint(asio::ip::make_address(this->ip), this->port));

    // TODO: what to do if failed to connect? do we keep retrying?
    this->socket_.connect(endpoint_, ec);
    
    if (ec) {
        return false;
    }

    return true;
}

void Client::send(std::uint8_t request) {
    boost::system::error_code ec;

    asio::write(this->socket_, asio::buffer(&request, sizeof(std::uint8_t)), ec);

    if (ec) {
        // TODO: what do we do if we fail to send a request? keep retrying or drop that request?
    }

}

std::vector<std::uint8_t> Client::recv(const int bufSize) {

    // TODO: not sure if this is very efficient or if there's a better way to do this

    // asio::streambuf recvbuf;
    boost::system::error_code ec;

    std::vector<std::uint8_t> recvbuf(bufSize);

    asio::read(this->socket_, asio::buffer(recvbuf), ec);

    if (ec) {
        // TODO: what to do when read fails
    }

    // recvbuf.commit(bufSize);
    
    return recvbuf;
}

std::string Client::getIP() {
    return this->ip;
}

int Client::getPort() {
    return this->port;
}



// #include "include/network/sync_client.hpp"

// CameraRequest_t Client::createRequest(RequestType_t requestType) {

//     CameraRequest_t request;
//     request.pid = 6969;
//     request.requestType = requestType;

//     return request;

// }

// void Client::createRequestPacket(RequestType_t requestType) {

//     CameraRequest_t request = createRequest(requestType);

//     serialh::serialize(&request, &this->sendbuf);

// }

// void Client::sendHeader() {

//     std::uint32_t headerLength = static_cast<std::uint32_t>(this->sendbuf.size());

//     // TODO: error handler
//     asio::write(this->socket_, asio::buffer(&headerLength, sizeof(headerLength)));

// }

// void Client::sendBody() {

//     // TODO: error handler
//     asio::write(this->socket_, this->sendbuf);

// }

// std::uint32_t Client::receiveHeader() {
    
//     std::uint32_t size {};

//     // TODO: error handler
//     asio::read(this->socket_, asio::buffer(&size, sizeof(size)));

//     return size;

// }

// void Client::receiveBody(std::uint32_t size) {

//     // TODO: error handler
//     asio::read(this->socket_, this->receivebuf.prepare(size));
//     this->receivebuf.commit(size);

// }

// CameraResponse_t Client::deconstructPacket() {

//     CameraResponse_t response;

//     serialh::deserialize(&response, &this->receivebuf);

//     return response;

// }

// void Client::handlePacket() {

//     // deconstruct packet
//     CameraResponse_t response = deconstructPacket();

//     std::vector<std::uint8_t> image = response.imageData.imgBuffer;
    
//     cv::Mat m = cv::Mat(response.imageData.height, response.imageData.width, CV_8UC3);

//     if (image.size() == (response.imageData.imageSizeBytes)) {
//         std::memcpy(m.data, image.data(), sizeof(std::uint8_t) * response.imageData.imageSizeBytes);
//     }

//     // Add image to image buffer
//     this->images.push_back(m);
// }

// Client::Client(asio::io_context* io_context_, std::string ip, int port) : socket_(*io_context_) {

//     this->ip = ip;
//     this->port = port;

// }

// void Client::connect() {

//     // TODO: difference between exception handling and error handling
//     try {

//         asio::ip::tcp::endpoint endpoint_(asio::ip::tcp::endpoint(asio::ip::make_address(this->ip), this->port));

//         this->socket_.connect(endpoint_);

//         std::cout << "Connected to: " << this->ip << " on port " << this->port << '\n';

//     } catch (std::exception& e) {

//         std::cout << e.what() << '\n';

//     }

// }

// void Client::send(RequestType_t requestType) {

//     createRequestPacket(requestType);

//     // send a header containing the length/size
//     sendHeader();

//     // send the serialized data in the streambuf
//     sendBody();

// }

// void Client::receive() {

//     std::uint32_t bodySize = receiveHeader();
//     receiveBody(bodySize);

//     handlePacket();

// }

// void Client::disconnect() {
    
//     // TODO: error handling
//     this->socket_.shutdown(asio::ip::tcp::socket::shutdown_both);
//     this->socket_.close();

// }

// void Client::showImages(){

//     for (cv::Mat img : this->images) {

//         std::string windowName = "Window";
//         cv::namedWindow(windowName);
//         cv::resizeWindow(windowName, 200, 200);
//         cv::imshow(windowName, img);
    
//         cv::waitKey(0);
    
//         cv::destroyWindow(windowName);
//     }
// }
