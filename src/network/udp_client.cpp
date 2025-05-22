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

std::vector<std::uint8_t> UDPClient::recvBody(const int mem_size, const int total_chunks) {
    boost::system::error_code ec;
    // asio::ip::udp::endpoint endpoint_(asio::ip::udp::endpoint(asio::ip::make_address(this->ip), this->port));
    asio::ip::udp::endpoint sender_endpoint;

    const int bufSize = mem_size * total_chunks;
    int totalBytesRead = 0;

    LOG_F(INFO, "Reading in bufSize (body): %d", bufSize);

    std::vector<std::uint8_t> buf(bufSize);

    int totalChunks = 0;

    for (int i = 0; i < total_chunks; i++) {
        char chunk_buf[CHUNK_SIZE + sizeof(uint32_t)];

        int bytesRead = this->socket_.receive_from(asio::buffer(chunk_buf), sender_endpoint, 0, ec);
        
        int chunk_idx = ntohl(*reinterpret_cast<uint32_t*>(chunk_buf));
        int data_size = bytesRead - sizeof(uint32_t);
        int offset = chunk_idx * CHUNK_SIZE;

        // copy into buffer
        memcpy(buf.data() + offset, chunk_buf + sizeof(uint32_t), data_size);

        totalBytesRead += data_size;
        totalChunks += 1;
        
        // LOG_F(INFO, "Chunk: %d, Total bytes read: %d, Total chunks: %d", chunk_idx, totalBytesRead, totalChunks);
    }

    if (ec) {
        LOG_F(ERROR, "Failed to send body: %s", ec.message().c_str());
        return {};
    }

    if (totalBytesRead != bufSize) {
        LOG_F(ERROR, "Total bytes read: %d, Expected bytes: %d", totalBytesRead, bufSize);
        return {}; // TODO: not sure what to return here, incomplete read
    }

    LOG_F(INFO, "Bytes read: %d", totalBytesRead);

    LOG_F(INFO, "Buffer size: %lu", buf.size());

    return buf;
}
