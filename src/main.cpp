#include <iostream>

#include "Eigen"
#include "core/states.hpp"
// #include <opencv2/opencv.hpp>
#include <mavsdk/mavsdk.h>
#include <torch/torch.h>

#include <iostream>

// using namespace cv;

int main() {
    PreparationState state;

    mavsdk::Mavsdk dc;
    mavsdk::ConnectionResult conn_result =
        dc.add_tcp_connection("127.0.0.1:5760");
    // Wait for the system to connect via heartbeat
    while (conn_result != mavsdk::ConnectionResult::Success) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Adding connection failed: " << conn_result << '\n';
    }

    state.tick();

    return 0;
}