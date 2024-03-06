#include "utilities/common.hpp"

#include <chrono>


std::chrono::seconds getUnixTime_s() {
    const auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
}

std::chrono::milliseconds getUnixTime_ms() {
    const auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
}
