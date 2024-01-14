#include <thread>
#include <chrono>
#include <iostream>
#include <mavsdk/mavsdk.h>

/*
* Test mavsdk connection to an endpoint 
*/ 
int main(int argc, char *argv[]) {
    // Print help blurb if no connection arg given
    if (argc == 1) {
        std::cout << "Usage:\n" 
                "bin/mavsdk tcp://127.0.0.1:5760\n"
                "bin/mavsdk [connection_link]\n"
                "connection_link -> [protocol]://[ip]:[port]\n";
        return 0;
    }

    mavsdk::Mavsdk mavsdk;
    mavsdk::ConnectionResult conn_result = mavsdk.add_any_connection(argv[1]);
    
    // Wait for the system to connect via heartbeat
    // TODO: add timeout
    if (conn_result != mavsdk::ConnectionResult::Success) {
        std::cout << "Adding connection failed: " << conn_result << '\n';
    }

    return 0;
}