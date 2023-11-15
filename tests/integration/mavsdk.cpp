#include <thread>
#include <chrono>
#include <iostream>
#include <mavsdk/mavsdk.h>

int main(int argc, char *argv[]) {
    
    mavsdk::Mavsdk dc;
    mavsdk::ConnectionResult conn_result = dc.add_tcp_connection(argv[1]);
    
    // Wait for the system to connect via heartbeat
    if (conn_result != mavsdk::ConnectionResult::Success) {
        std::cout << "Adding connection failed: " << conn_result << '\n';
    }

    return 0;
}