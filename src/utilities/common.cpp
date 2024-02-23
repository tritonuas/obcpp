#include "utilities/common.hpp"

#include <chrono>

std::chrono::seconds getUnixTime() {
    const auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
}
