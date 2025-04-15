#include <iostream>
#include <boost/asio.hpp>
#include "include/network/async_client.hpp"
#include "include/utilities/locks.hpp"
#include "include/network/serialize.hpp"

namespace asio = boost::asio;

void Client::createRequestPacket(RequestType_t requestType) {

    // form the data
    CameraRequest_t request;
    request.requestType = requestType;
    request.pid = 1010; // TODO: change the PID or completely remove PID

    // serialize the data
    serialh::serialize(&request, &this->sendbuf);

}

void Client::send(RequestType_t requestType) {

    // TODO: probably need to lock here

    this->createRequestPacket(requestType);

    // two sends here, first send the size of the serialized data, then send the serialized data itself
    // TODO: is a chained callback structure the best way to do this?

    std::uint32_t headerLength = static_cast<std::uint32_t>(this->sendbuf.size());

    asio::async_write(this->socket_, asio::buffer(&headerLength, sizeof(headerLength)), 
        [this](boost::system::error_code ec, std::size_t bytes_written) {

            if (ec) {
                // TODO: what do we do if sending the header fails?
            }

            asio::async_write(this->socket_, this->sendbuf, 
                [this](boost::system::error_code ec, std::size_t bytes_written) {

                    if (ec) {
                        // TODO: what do we do if sending the body fails?
                    }

                });
        });

}

void Client::recv() {

    // TODO: maybe make this shared as well
    std::uint32_t headerLength = {};

    asio::async_read(this->socket_, asio::buffer(&headerLength, sizeof(headerLength)), 
        [this, headerLength](boost::system::error_code ec, std::size_t bytes_read) {
            
            if (ec) {
                // TODO: what to do if sending header length fails, best guess is to not send the packet at all
            }

            asio::streambuf buf;

            std::shared_ptr<asio::streambuf> recvbuf {&buf};

            asio::async_read(this->socket_, recvbuf->prepare(headerLength), 
                [this, headerLength, recvbuf](boost::system::error_code ec, std::size_t bytes_read) {

                    if (ec || bytes_read != headerLength) {

                        // we read some data but not all of it so its likely not usable
                        recvbuf->consume(bytes_read);

                        // TODO: what do we do 
                    }

                    recvbuf->commit(bytes_read);

                    cv::Mat m = this->imgConvert(); // TODO: have to pass the recvbuf through here

                    this->images.push(m);

                });

        });

}

cv::Mat Client::imgConvert() {

    // TODO: apparently the expected format will be YUV420, look into what that is. can be done with ffmpeg per Daniel. the current code just assumes that what was sent is in jpeg format or whatever, not YUV420
    // using opencv cvtColor(src, dst, cv::COLOR_YUV2BGR_NV21)?
    // will this function be reading in images and then converting them? or are we converting them as we receive the image buffers


    // this logic is what deconstructPacket() was supposed to be
    CameraResponse_t response;

    serialh::deserialize(&response, &this->recvbuf);

    std::vector<std::uint8_t> image = response.imageData.imgBuffer;

    // "reassemble" the image
    cv::Mat m = cv::Mat(response.imageData.height, response.imageData.width, CV_8UC3);

    if (image.size() == (response.imageData.imageSizeBytes)) {
        std::memcpy(m.data, image.data(), sizeof(std::uint8_t) * response.imageData.imageSizeBytes);
    }

    return m;

}

Client::Client(asio::io_context* io_context_, std::string ip, int port) : socket_(*io_context_) {

    this->ip = ip;
    this->port = port;

    // TODO: could use the socket keepalive option here to keep a persistent connection, unsure how it differs from the ping() function. idk what the ping() function even does.

}

void Client::connect() {

    // TODO: should we check if its already connected? maybe multiple threads could try calling connect()

    // make the endpoint
    asio::ip::tcp::endpoint endpoint_(asio::ip::tcp::endpoint(asio::ip::make_address(this->ip), this->port));

    // asynchronously connect to endpoint
    // TODO: is a lambda appropriate here as the completion handler?
    this->socket_.async_connect(endpoint_, 
        [this](boost::system::error_code ec) {

            // TODO: what do we do in case of an error?
            if (ec) {
                std::cout << "Failed to connect" << '\n';
                std::cout << "Reason: " << ec.message() << " code: " << ec.value() << '\n';
            }

            std::cout << "Connected to: " << this->ip << " on port: " << this->port << '\n';

        });

}

void Client::ping() {

    /*
        This is what i assume the function does:
        - basically just open up a new connection and send a packet with Ping + other stuff and wait for a response back
        - if it fails to connect or a response is taking too long, then something has happened but what do we do? print out a request timed out message?
    */

}

void Client::takePicture() {

    this->send(RequestType_t::SENDIMAGE);

    this->recv();

}

cv::Mat Client::getLatestImage() {

    // TODO: what if there are no images in the image buffer?

    cv::Mat latestImage {};

    // TODO: acquire a lock, just regular not a read or write lock?

    latestImage = this->images.front();
    this->images.pop();

    // TODO: release the lock

    return latestImage;

}



