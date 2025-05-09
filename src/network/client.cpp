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

Header Client::recvHeader() {
    boost::system::error_code ec;
    Header header;

    // TODO: might have to specify 12 bytes
    asio::read(this->socket_, asio::buffer(&header, sizeof(Header)), ec);

    return header;
}

std::vector<std::uint8_t> Client::recvBody(const int bufSize) {

    boost::system::error_code ec;

    std::vector<std::uint8_t> recvbuf(bufSize);

    asio::read(this->socket_, asio::buffer(recvbuf), ec);

    if (ec) {
        // TODO: what to do when read fails
    }
    
    return recvbuf;
}

std::string Client::getIP() {
    return this->ip;
}

int Client::getPort() {
    return this->port;
}
