#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "core/airdrop_retry.hpp"

TEST(AirdropRetryTest, WaitsThreeSecondsAfterFailureBeforeRetry) {
    using Clock = std::chrono::steady_clock;
    std::vector<Clock::time_point> attemptTimes;
    int attempts = 0;
    int loggedFailures = 0;

    auto makeSocket = [&]() {
        attemptTimes.push_back(Clock::now());

        ad_socket_result_t result{};
        if (attempts++ == 0) {
            result.is_err = 1;
            result.data.err = "simulated bind failure";
        } else {
            result.data.res.send_port = 0;
            result.data.res.recv_port = 0;
            result.data.res.fd = -1;
        }
        return result;
    };

    auto result = airdrop_retry::createSocketWithRetry(
        makeSocket,
        [&loggedFailures](const char*) { ++loggedFailures; },
        [](std::chrono::seconds delay) { std::this_thread::sleep_for(delay); });

    ASSERT_FALSE(result.is_err);
    ASSERT_EQ(attempts, 2);
    ASSERT_EQ(loggedFailures, 1);
    ASSERT_EQ(attemptTimes.size(), 2);

    const auto elapsed = attemptTimes[1] - attemptTimes[0];
    const auto elapsedMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    std::cout << "[retry timing evidence] failed attempt to next attempt: "
              << elapsedMs << " ms\n";
    EXPECT_GE(elapsed, airdrop_retry::kSocketRetryDelay);
}
