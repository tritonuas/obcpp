#ifndef INCLUDE_NETWORK_RPI_CONNECTION_HPP_
#define INCLUDE_NETWORK_RPI_CONNECTION_HPP_

#include <iostream>

const std::string SERVER_IP = "192.168.68.1";
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
