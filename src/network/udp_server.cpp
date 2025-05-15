#include "network/udp_server.hpp"

const uint32_t IMG_WIDTH = 2028;
const uint32_t IMG_HEIGHT = 1520;
const uint32_t BUFFER_SIZE = IMG_WIDTH * IMG_HEIGHT * 3 / 2;

void UDPServer::takePicture() {

}



UDPServer::UDPServer(asio::io_context* io_context_, std::string ip, int port) : socket_(*io_context_){

}

void UDPServer::start() {

}

void UDPServer::send() {

}

void UDPServer::recv() {

}

void UDPServer::handleRequest(char request) {

}

void UDPServer::shutdown() {

}
