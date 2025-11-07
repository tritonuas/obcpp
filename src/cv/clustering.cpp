#include "cv/clustering.hpp"
#include <vector>
#include "protos/obc.pb.h"
#include "cv/localization.hpp"
#include <algorithm>
#include <tuple>
#include "cv/utilities.hpp"
#include "camera/interface.hpp"
#include "utilities/datatypes.hpp"


/*
 Takes in a list of list of points, where each list is a set of points in one cluster

 uses median approach
 */
std::vector<GPSCoord> Clustering::FindClustersCenter(const std::vector<std::vector<GPSCoord>>& points){
        GSDLocalization local;
        std::vector<GPSCoord> centers;
        centers.reserve(points.size());
        for(const std::vector<GPSCoord> &cluster : points){
            GPSCoord avg;
            std::vector<double> lats;
            std::vector<double> longs;
            lats.reserve(cluster.size());
            longs.reserve(cluster.size());
            for(const GPSCoord& cord : cluster){
                lats.push_back(cord.latitude());
                longs.push_back(cord.longitude());
            }
            std::sort(lats.begin(), lats.end());
            std::sort(longs.begin(), longs.end());
            avg.set_latitude(lats[lats.size()/2]);
            avg.set_longitude(longs[longs.size()/2]);
            centers.push_back(std::move(avg));
        }
        return std::move(centers);
    }
 //mean approach 
// std::vector<GPSCoord> Clustering::FindClustersCenter(const std::vector<std::vector<GPSCoord>>& points){
//     std::vector<GPSCoord> centers;
//     centers.reserve(points.size());
//     for(std::vector<GPSCoord> &cluster : points){
//         GPSCoord avg;
//         for(GPSCoord cord : cluster){
//             avg.set_latitude(avg.latitude() + cord.latitude());
//             avg.set_longitude(avg.longitude() + cord.longitude());
//             avg.set_altitude(avg.altitude() + cord.altitude());
//         }
//         avg.set_latitude(avg.latitude()/cluster.size());
//         avg.set_longitude(avg.longitude()/cluster.size());
//         avg.set_altitude(avg.altitude()/cluster.size());
//         centers.push_back(std::move(avg));
//     }
//     return std::move(centers);
// }