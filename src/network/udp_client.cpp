#include "network/udp_client.hpp"
#include <sys/select.h>
#include <chrono>

UDPClient::UDPClient(asio::io_context* io_context_, std::string ip, int port)
    : socket_(*io_context_) {
    this->ip = ip;
    this->port = port;
}

bool UDPClient::connect() {
    boost::system::error_code ec;

    // Open the UDP socket
    this->socket_.open(asio::ip::udp::v4(), ec);

    if (ec) {
        LOG_F(ERROR, "Failed to connect");
        return false;
    }

    // Set Receive Timeout
    struct timeval read_timeout;
    read_timeout.tv_sec = 2;
    read_timeout.tv_usec = 0;
    setsockopt(this->socket_.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &read_timeout,
               sizeof(read_timeout));

    LOG_F(INFO, "Connected to %s on port %d", this->ip.c_str(), this->port);
    return true;
}

void UDPClient::setReceiveTimeout(int timeout_ms) {
    this->current_timeout_ms_ = timeout_ms;
    struct timeval read_timeout;
    read_timeout.tv_sec = timeout_ms / 1000;
    read_timeout.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(this->socket_.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &read_timeout,
               sizeof(read_timeout));
}

bool UDPClient::waitForData() {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(this->socket_.native_handle(), &fds);

    struct timeval tv;
    tv.tv_sec = this->current_timeout_ms_ / 1000;
    tv.tv_usec = (this->current_timeout_ms_ % 1000) * 1000;

    int rv = select(this->socket_.native_handle() + 1, &fds, NULL, NULL, &tv);

    if (rv == -1) {
        LOG_F(ERROR, "Select failed in waitForData.");
        return false;
    } else if (rv == 0) {
        // Timeout
        return false;
    }
    return true;
}

// send a request to the server
bool UDPClient::send(std::uint8_t request) {
    boost::system::error_code ec;
    asio::ip::udp::endpoint endpoint_(
        asio::ip::udp::endpoint(asio::ip::make_address(this->ip), this->port));

    LOG_F(INFO, "Sending request %c", request);

    int bytesSent = this->socket_.send_to(asio::buffer(&request, sizeof(request)),
                                          endpoint_, 0, ec);

    if (ec) {
        LOG_F(WARNING, "Failed to send request: %s", ec.message().c_str());
        return false;
    }

    return true;
}

// receive an image back from the server
Header UDPClient::recvHeader() {
    boost::system::error_code ec;
    asio::ip::udp::endpoint sender_endpoint;
    Header header;

    if (!this->waitForData()) {
        LOG_F(WARNING, "Timeout waiting for header data.");
        return {};
    }

    int bytesRead = this->socket_.receive_from(asio::buffer(&header, sizeof(Header)),
                                               sender_endpoint, 0, ec);

    if (ec) {
        LOG_F(ERROR, "Failed to read header: %s", ec.message().c_str());
        return {};
    }

    header.magic = ntohl(header.magic);
    header.total_chunks = ntohl(header.total_chunks);
    header.mem_size = ntohl(header.mem_size);

    return header;
}

std::vector<std::uint8_t> UDPClient::recvBody(const int mem_size, const int total_chunks) {
    boost::system::error_code ec;
    asio::ip::udp::endpoint sender_endpoint;

    const int bufSize = mem_size;
    std::vector<std::uint8_t> buf(bufSize);
    std::vector<bool> received_chunks(total_chunks, false);

    int chunks_received_count = 0;

    // Use raw buffer? hopefully no mem leaks
    char chunk_buf[CHUNK_SIZE + sizeof(uint32_t)];

    // Loop until all chunks received
    // Note: we prolly want to add a max retry count or timeout check inside the loop
    // But the socket timeout will break the loop eventually if data stops coming
    while (chunks_received_count < total_chunks) {
        if (!this->waitForData()) {
             LOG_F(WARNING, "Timeout waiting for body chunks.");
             return {};
        }

        size_t bytesRead = this->socket_.receive_from(asio::buffer(chunk_buf),
                                                      sender_endpoint, 0, ec);

        if (ec) {
             LOG_F(ERROR, "Receive chunk failed: %s", ec.message().c_str());
             // Return what we have or empty? Returning empty signals failure.
             return {};
        }

        if (bytesRead < sizeof(uint32_t)) continue;

        uint32_t chunk_idx = ntohl(*reinterpret_cast<uint32_t*>(chunk_buf));
        size_t data_size = bytesRead - sizeof(uint32_t);
        size_t offset = chunk_idx * CHUNK_SIZE;

        if (offset + data_size > static_cast<size_t>(bufSize)) {
             LOG_F(ERROR, "Chunk exceeds buffer bounds");
             continue;
        }

        // copy into buffer
        memcpy(buf.data() + offset, chunk_buf + sizeof(uint32_t), data_size);

        if (!received_chunks[chunk_idx]) {
            received_chunks[chunk_idx] = true;
            chunks_received_count++;
        }
    }

    LOG_F(INFO, "Successfully reconstructed plane: %lu bytes", buf.size());
    return buf;
}

char UDPClient::recvPing() {
    boost::system::error_code ec;
    asio::ip::udp::endpoint sender_endpoint;
    char c = 0;

    if (!this->waitForData()) {
        LOG_F(WARNING, "Timeout waiting for ping response");
        return 0;
    }

    size_t n = this->socket_.receive_from(asio::buffer(&c, 1), sender_endpoint, 0, ec);
    if (ec || n != 1) {
        LOG_F(ERROR, "Failed to receive ping byte: %s", ec.message().c_str());
        return 0;
    }

    return c;
}