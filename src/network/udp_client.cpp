#include "network/udp_client.hpp"

UDPClient::UDPClient(asio::io_context* io_context_, std::string ip, int port) : socket_(*io_context_) {
    this->ip = ip;
    this->port = port;
}

bool UDPClient::connect() {
    boost::system::error_code ec;
    asio::ip::udp::endpoint endpoint_(asio::ip::udp::endpoint(asio::ip::make_address(this->ip), this->port));

    // open the UDP socket using ip::udp::v4()
    this->socket_.open(asio::ip::udp::v4(), ec);

    if (ec) {
        LOG_F(ERROR, "Failed to connect");
        return false;
    }

    LOG_F(INFO, "Connected to %s on port %d", this->ip.c_str(), this->port);

    return true;
}

// send a request to the server
bool UDPClient::send(std::uint8_t request) {
    boost::system::error_code ec;
    asio::ip::udp::endpoint endpoint_(asio::ip::udp::endpoint(asio::ip::make_address(this->ip), this->port));

    LOG_F(INFO, "Sending request %c", request);

    int bytesSent = this->socket_.send_to(asio::buffer(&request, sizeof(request)), endpoint_, 0, ec);

    if (ec) {
        LOG_F(ERROR, "Failed to send request: %s", ec.message().c_str());
        return false;
    }

    LOG_F(INFO, "Sent %d bytes", bytesSent);

    return true;
}

// receive an image back from the server
Header UDPClient::recvHeader() {
    boost::system::error_code ec;
    // asio::ip::udp::endpoint endpoint_(asio::ip::udp::endpoint(asio::ip::make_address(this->ip), this->port));
    asio::ip::udp::endpoint sender_endpoint;

    Header header;

    int bytesRead = this->socket_.receive_from(asio::buffer(&header, sizeof(Header)), sender_endpoint, 0, ec);

    if (ec) {
        LOG_F(ERROR, "Failed to read header: %s", ec.message().c_str());
        return {};
    }

    LOG_F(INFO, "Bytes read (header): %d", bytesRead);

    return header;
}

std::vector<std::uint8_t> UDPClient::recvBody(const int bufSize) {
    boost::system::error_code ec;
    // asio::ip::udp::endpoint endpoint_(asio::ip::udp::endpoint(asio::ip::make_address(this->ip), this->port));
    asio::ip::udp::endpoint sender_endpoint;

    LOG_F(INFO, "Reading in bufSize (body): %d", bufSize);

    std::vector<std::uint8_t> buf(bufSize);

    int bytesRead = this->socket_.receive_from(asio::buffer(buf), sender_endpoint, 0, ec);

    if (ec) {
        LOG_F(ERROR, "Failed to send body: %s", ec.message().c_str());
        return {};
    }

    LOG_F(INFO, "Bytes read: %d", bytesRead);

    return buf;
}
