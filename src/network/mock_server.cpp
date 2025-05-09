#include "network/mock_server.hpp"

const uint32_t IMG_WIDTH = 2028;
const uint32_t IMG_HEIGHT = 1520;
const uint32_t BUFFER_SIZE = IMG_WIDTH * IMG_HEIGHT * 3 / 2;

Server::Server(asio::io_context* io_context_, std::string ip, int port) : socket_(*io_context_), acceptor_(*io_context_, asio::ip::tcp::endpoint(asio::ip::make_address(ip), port))  {
    this->ip = ip;
    this->port = port;
}

void Server::start() {

    boost::system::error_code ec;

    this->acceptor_.accept(this->socket_, ec);

    if (ec) {
        std::cout << "Error code: " << ec.value() << '\n';
    }

    std::cout << "Listening on port: " << this->port << '\n';

    recv();
}

cv::Mat Server::createBGR() {
    cv::Mat image = cv::imread("blurb.jpeg");
    if (image.empty()) {
        std::cerr << "Failed to load image. Check path: blurb.jpeg\n";
        exit(1);  // or return a default image
    }
    std::cout << "Image is not empty" << '\n';
    return image;
}

cv::Mat Server::createYUV() {
    // read in an image
    cv::Mat image = cv::imread("blurb.jpeg");

    // convert into yuv420
    cv::Mat yuv_img(IMG_HEIGHT * 3 / 2, IMG_WIDTH, CV_8UC1); 
    cv::cvtColor(image, yuv_img, cv::COLOR_BGR2YUV_I420);

    return yuv_img;
}

void Server::send() {
    boost::system::error_code ec;

    std::cout << "creating the image" << '\n';

    // cv::Mat img = createBGR();
    cv::Mat img = createYUV();

    std:: cout << "putting image into buffer" << '\n';

    // put the image into a buffer
    cv::Mat flatten = img.reshape(1, img.total() * img.channels());
    std::vector<std::uint8_t> imgBuffer = img.isContinuous() ? flatten : flatten.clone();

    // check the size of the buffer
    if (imgBuffer.size() != BUFFER_SIZE) {
        std::cout << "size: " << imgBuffer.size() << " expected: " << BUFFER_SIZE << '\n';
        return;
    }

    // write the buffer
    asio::write(this->socket_, asio::buffer(imgBuffer), ec);

    if (ec) {
        std::cout << "server send failed" << '\n';
    }
}

void Server::recv() {
    boost::system::error_code ec;

    // read in request, will be a char

    char request;

    asio::read(this->socket_, asio::buffer(&request, sizeof(char)), ec);

    if (ec) {
        // TODO: what to do if can't read the request
    }

    std::cout << "Request received: " << request << '\n';
    handleRequest(request);
}

void Server::handleRequest(char request) {
    if (request == 'p') {
        // take a picture
        this->send();
    } else if (request == 'e') {
        // close the connection
        this->shutdown();
    }
}

void Server::shutdown() {
    std::cout << "shutting down" << '\n';
}
