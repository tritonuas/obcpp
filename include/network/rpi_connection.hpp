#ifndef INCLUDE_NETWORK_RPI_CONNECTION_HPP_
#define INCLUDE_NETWORK_RPI_CONNECTION_HPP_

#include <iostream>

// Image Config
const uint32_t IMG_WIDTH = 1456;
const uint32_t IMG_HEIGHT = 1088;
const uint32_t IMG_BUFFER = IMG_WIDTH * IMG_HEIGHT * 3 / 2;

// Libcamera Strides/Padding
const uint32_t STRIDE_Y = 1472;
const uint32_t STRIDE_UV = 736;

// Network Config
const std::string SERVER_IP = "192.168.77.2";
const int SERVER_PORT = 25565;

const int headerSize = 12;
const uint32_t EXPECTED_MAGIC = 0x12345678;
const size_t CHUNK_SIZE = 1024;

struct Header {
    uint32_t magic;
    uint32_t total_chunks;
    uint32_t mem_size;
};

#endif