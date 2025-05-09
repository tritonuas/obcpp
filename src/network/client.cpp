#include <loguru.hpp>
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
        LOG_F(INFO, std::string("Failed to connect: " + ec.message()).c_str());
        return false;
    }

    LOG_F(INFO, std::string("Connected to: " + this->ip + " on port: " + std::to_string(this->port)).c_str());

    return true;
}

void Client::send(std::uint8_t request) {
    boost::system::error_code ec;

    LOG_F(INFO, std::string("Sending request" + static_cast<char>(request)).c_str());

    int bytesSent = asio::write(this->socket_, asio::buffer(&request, sizeof(std::uint8_t)), ec);

    if (ec) {
        // TODO: what do we do if we fail to send a request? keep retrying or drop that request?
        LOG_F(INFO, std::string("Failed to send request: " + ec.message()).c_str());
    }

    LOG_F(INFO, std::string("Bytes sent: " + bytesSent).c_str());

}

Header Client::recvHeader() {
    boost::system::error_code ec;
    Header header;

    // TODO: might have to specify 12 bytes
    int bytesRead = asio::read(this->socket_, asio::buffer(&header, sizeof(Header)), ec);

    if (ec) {        
        // TODO: what to do when read fails
        LOG_F(INFO, std::string("Failed to read header: " + ec.message()).c_str());
    }

    LOG_F(INFO, std::string("Bytes read (header): " + bytesRead).c_str());

    return header;
}

std::vector<std::uint8_t> Client::recvBody(const int bufSize) {

    boost::system::error_code ec;

    LOG_F(INFO, std::string("Reading in bufSize (body): " + std::to_string(bufSize)).c_str());

    std::vector<std::uint8_t> recvbuf(bufSize);

    int bytesRead = asio::read(this->socket_, asio::buffer(recvbuf), ec);

    if (ec) {        
        // TODO: what to do when read fails
        LOG_F(INFO, std::string("Failed to read body: " + ec.message()).c_str());
    }

    LOG_F(INFO, std::string("Bytes read (body): " + bytesRead).c_str());
    
    return recvbuf;
}

std::string Client::getIP() {
    return this->ip;
}

int Client::getPort() {
    return this->port;
}
