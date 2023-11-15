#include <thread>
#include <chrono>
#include <iostream>
#include <mavsdk/mavsdk.h>


/*
* Test mavsdk connection to an endpoint 
* bin/mavsdk [connection_link]
* connection_link -> [protocol]://[ip]:[port]
* example: 
*   bin/mavsdk tcp://127.0.0.1:5760
*/ 
int main(int argc, char *argv[]) {
    
    mavsdk::Mavsdk dc;
    mavsdk::ConnectionResult conn_result = dc.add_any_connection(argv[1]);
    
    // Wait for the system to connect via heartbeat
    if (conn_result != mavsdk::ConnectionResult::Success) {
        std::cout << "Adding connection failed: " << conn_result << '\n';
    }

    return 0;
}