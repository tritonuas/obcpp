#ifndef INCLUDE_CORE_AIRDROP_RETRY_HPP_
#define INCLUDE_CORE_AIRDROP_RETRY_HPP_

#include <chrono>

extern "C" {
#include "network/airdrop_sockets.h"
}

namespace airdrop {

inline constexpr std::chrono::seconds kSocketRetryDelay{3};

// The callbacks keep the retry policy testable without real network hardware.
template <typename SocketFactory, typename FailureLogger, typename Sleeper>
ad_socket_result_t createSocketWithRetry(
    SocketFactory makeSocket,
    FailureLogger logFailure,
    Sleeper sleep) {
    while (true) {
        ad_socket_result_t result = makeSocket();
        if (!result.is_err) {
            return result;
        }

        logFailure(result.data.err);
        sleep(kSocketRetryDelay);
    }
}

}  // namespace airdrop

#endif  // INCLUDE_CORE_AIRDROP_RETRY_HPP_
