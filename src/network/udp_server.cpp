#include <filesystem>
#include "network/udp_server.hpp"

// TODO: Didn't touch this for the most part since its for mocking. 

cv::Mat UDPServer::createBGR() {
    std::cout << std::filesystem::current_path() << '\n';
    cv::Mat image = cv::imread("/workspaces/obcpp/tests/integration/images/blurb.jpeg");
    if (image.empty()) {
        std::cerr << "Failed to load image. Check path: blurb.jpeg\n";
        exit(1);  // or return a default image
    }
    std::cout << "Image is not empty" << '\n';
    return image;
}

cv::Mat UDPServer::createYUV() {
    std::cout << std::filesystem::current_path() << '\n';
    // read in an image
    cv::Mat image = cv::imread("/workspaces/obcpp/tests/integration/images/blurb.jpeg");

    // convert into yuv420
    cv::Mat yuv_img(IMG_HEIGHT * 3 / 2, IMG_WIDTH, CV_8UC1); 
    cv::cvtColor(image, yuv_img, cv::COLOR_BGR2YUV_I420);

    return yuv_img;
}

UDPServer::UDPServer(asio::io_context* io_context_, std::string ip, int port) : socket_(*io_context_){
    this->ip = ip;
    this->port = port;
}

bool UDPServer::start() {
    boost::system::error_code open_ec;
    boost::system::error_code bind_ec;
    asio::ip::udp::endpoint endpoint_(asio::ip::udp::endpoint(asio::ip::make_address(this->ip), this->port));

    std::cout << "Attempting to connect to " << this->ip << " on port " <<  this->port << '\n';

    // open the udp socket
    this->socket_.open(asio::ip::udp::v4(), open_ec);

    if (open_ec) {
        std::cout << "Failed to open socket: " << open_ec.message() << '\n';
        return false;
    }
    
    // bind to this address and port
    this->socket_.bind(endpoint_, bind_ec);
    
    std::cout << "Endpoint: " << endpoint_.address() << endpoint_.port() <<
    '\n';

    if (bind_ec) {
        std::cout << "Failed to bind socket: " << bind_ec.message() << '\n';
        return false;
    }

    return true;
}

void UDPServer::send(asio::ip::udp::endpoint & endpoint) {
    // Note: use the endpoint from recv() to send back

    boost::system::error_code header_ec;
    boost::system::error_code body_ec;

    std::cout << "Taking picture (reads in image)" << '\n';

    // cv::Mat img = createBGR();
    cv::Mat img = createYUV();

    std:: cout << "Putting image into buffer" << '\n';

    cv::Mat flatten = img.reshape(1, img.total() * img.channels());
    std::vector<std::uint8_t> imgBuffer = img.isContinuous() ? flatten : flatten.clone();

    if (imgBuffer.size() != IMG_BUFFER) {
        std::cout << "size: " << imgBuffer.size() << " expected: " << IMG_BUFFER << '\n';
        return;
    }

    uint32_t total_chunks = imgBuffer.size() / CHUNK_SIZE;

    Header header;

    header.magic = htonl(EXPECTED_MAGIC);
    header.mem_size = htonl(CHUNK_SIZE);
    header.total_chunks = htonl(total_chunks);

    // send header
    int bytesSentHeader = this->socket_.send_to(asio::buffer(&header, sizeof(header)), endpoint, 0, header_ec);

    if (header_ec) {
        std::cout << "Sending header failed: " << header_ec.message() << '\n';
        return;
    }

    std::cout << "Read bytes (header): " << bytesSentHeader << '\n';

    // send body

    const int buf_size = imgBuffer.size();

    std::cout << "Total chunks: " << imgBuffer.size() / CHUNK_SIZE << '\n';

    int totalBytesSent = 0;

    for (uint32_t i = 0; i < total_chunks; i++) {
        const size_t offset = i * CHUNK_SIZE;
        const size_t remaining = buf_size - offset;
        const size_t data_size = std::min(CHUNK_SIZE, remaining);

        std::vector<char> packet(sizeof(uint32_t) + data_size);

        // puts chunk index as the header
        uint32_t* index_ptr = reinterpret_cast<uint32_t*>(packet.data());
        *index_ptr = htonl(i);


        memcpy(packet.data() + sizeof(uint32_t), imgBuffer.data() + offset, data_size);

        std::this_thread::sleep_for(std::chrono::microseconds(50));
        int bytesSentBody = this->socket_.send_to(asio::buffer(packet), endpoint, 0, body_ec);

        totalBytesSent += (bytesSentBody - sizeof(uint32_t));

        if (body_ec) {
            std::cout << "Sending body failed: " << body_ec.message()  << '\n';
            return;
        }

    }

    std::cout << "Finished sending " << totalBytesSent << " bytes" << '\n';

    // int bytesSentBody = this->socket_.send_to(asio::buffer(imgBuffer), endpoint, 0, body_ec);

    // std::cout << "Read bytes (body): " << bytesSentBody << '\n';

}

// expecting a request from the client
void UDPServer::recv() {
    // 
    boost::system::error_code ec;
    asio::ip::udp::endpoint client_endpoint;

    char request;

    int bytesRead = this->socket_.receive_from(asio::buffer(&request, sizeof(request)), client_endpoint, 0, ec);

    if (ec) {
        std::cout << "Failed to read request: " << ec.message() << '\n';
    }

    std::cout << "Bytes read (request): " << bytesRead << '\n';

    handleRequest(request, client_endpoint);
}

void UDPServer::handleRequest(char request, asio::ip::udp::endpoint & endpoint) {
    if (request == 'I') {
        this->send(endpoint);
    } else if (request == 'e') {
        this->shutdown();
    } else {
        std::cout << "Invalid request: " << request << '\n';
    }
}

void UDPServer::shutdown() {
    std::cout << "shutting down" << '\n';
}

// cv::Mat UDPServer::createBGR() {
//     cv::Mat image = cv::imread("blurb.jpeg");
//     if (image.empty()) {
//         std::cerr << "Failed to load image. Check path: blurb.jpeg\n";
//         exit(1);  // or return a default image
//     }
//     std::cout << "Image is not empty" << '\n';
//     return image;
// }

// cv::Mat UDPServer::createYUV() {
//     // read in an image
//     cv::Mat image = cv::imread("blurb.jpeg");

//     // convert into yuv420
//     cv::Mat yuv_img(IMG_HEIGHT * 3 / 2, IMG_WIDTH, CV_8UC1); 
//     cv::cvtColor(image, yuv_img, cv::COLOR_BGR2YUV_I420);

//     return yuv_img;
// }

// UDPServer::UDPServer(asio::io_context* io_context_, std::string ip, int port) : socket_(*io_context_){
//     this->ip = ip;
//     this->port = port;
// }

// bool UDPServer::start() {
//     boost::system::error_code open_ec;
//     boost::system::error_code bind_ec;
//     asio::ip::udp::endpoint endpoint_(asio::ip::udp::endpoint(asio::ip::make_address(this->ip), this->port));

//     std::cout << ("Attempting to connect to %s on port %d", this->ip, this->port) << '\n';

//     // open the udp socket
//     this->socket_.open(asio::ip::udp::v4(), open_ec);

//     if (open_ec) {
//         std::cout << ("Failed to open socket: %s", open_ec.message()) << '\n';
//         return false;
//     }
    
//     // bind to this address and port
//     this->socket_.bind(endpoint_, bind_ec);

//     if (bind_ec) {
//         std::cout << ("Failed to bind socket: %s", bind_ec.message()) << '\n';
//         return false;
//     }

//     return true;
// }

// void UDPServer::send(asio::ip::udp::endpoint & endpoint) {
//     // Note: use the endpoint from recv() to send back

//     boost::system::error_code header_ec;
//     boost::system::error_code body_ec;

//     std::cout << "Taking picture (reads in image)" << '\n';

//     // cv::Mat img = createBGR();
//     cv::Mat img = createYUV();

//     std:: cout << "Putting image into buffer" << '\n';

//     cv::Mat flatten = img.reshape(1, img.total() * img.channels());
//     std::vector<std::uint8_t> imgBuffer = img.isContinuous() ? flatten : flatten.clone();

//     if (imgBuffer.size() != BUFFER_SIZE) {
//         std::cout << "size: " << imgBuffer.size() << " expected: " << BUFFER_SIZE << '\n';
//         return;
//     }

//     Header header;

//     header.magic = htonl(EXPECTED_MAGIC);
//     header.mem_size = htonl(CHUNK_SIZE);
//     header.total_chunks = htonl(imgBuffer.size() / CHUNK_SIZE);

//     // send header
//     int bytesReadHeader = this->socket_.send_to(asio::buffer(&header, sizeof(header)), endpoint, 0, header_ec);

//     if (header_ec) {
//         std::cout << ("Sending header failed: %s", header_ec.message()) << '\n';
//         return;
//     }

//     // send body
//      int bytesReadBody = this->socket_.send_to(asio::buffer(imgBuffer), endpoint, 0, body_ec);

//     if (body_ec) {
//         std::cout << ("Sending body failed: %s", body_ec.message())  << '\n';
//         return;
//     }

// }

// // expecting a request from the client
// void UDPServer::recv() {
//     // 
//     boost::system::error_code ec;
//     asio::ip::udp::endpoint client_endpoint;

//     char request;

//     int bytesRead = this->socket_.receive_from(asio::buffer(&request, sizeof(request)), client_endpoint, 0, ec);

//     if (ec) {
//         std::cout << ("Failed to read request: %s", ec.message()) << '\n';
//     }

//     std::cout << ("Bytes read (request): %d", bytesRead) << '\n';

//     handleRequest(request, client_endpoint);
// }

// void UDPServer::handleRequest(char request, asio::ip::udp::endpoint & endpoint) {
//     if (request == 'p') {
//         this->send(endpoint);
//     } else if (request == 'e') {
//         this->shutdown();
//     } else {
//         std::cout << ("Invalid request: %c", request) << '\n';
//     }
// }

// void UDPServer::shutdown() {
//     std::cout << "shutting down" << '\n';
// }
