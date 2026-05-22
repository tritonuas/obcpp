#include <iostream> 
#include <chrono>
#include <utility>
#include <vector>

#include "utilities/datatypes.hpp"
#include "pathing/mission_path.hpp"
#include "ticks/path_gen.hpp"
#include "pathing/mission_path.hpp"
#include "ticks/path_validate.hpp"
#include "network/gcs_routes.hpp"
#include "protos/obc.pb.h"


std::chrono::seconds getUnixTime_s() {
    const auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
}

std::chrono::milliseconds getUnixTime_ms() {
    const auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
}

int checkCounterClockwise(std::vector<GPSCoord> coords) {
   double tot_sign = 0; 
   double xnplus1 = 0; 
   double ynplus1 = 0;
   for(int i = 0; i < coords.size(); i++){
    double xn = coords[i].latitude(); 
    double yn = coords[i].longitude();
    if(i == coords.size() - 1){
        xnplus1 = coords[0].latitude(); 
        ynplus1 =  coords[0].longitude(); 
    } else {
        xnplus1 = coords[i+1].latitude(); 
        ynplus1 =  coords[i+1].longitude(); 
    }
    tot_sign += ((xn *ynplus1) - (yn * xnplus1)); 
   }
   std::cout <<  tot_sign; 
   if(tot_sign < 0){
    return 0; // 0 is counter clockwise
   }else {
    return 1; // 1 is not counterclockwise
   }
}
//checks coordinates if they're in counter clockwise