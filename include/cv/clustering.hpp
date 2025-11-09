#ifndef INCLUDE_CV_CLUSTERING_HPP_
#define INCLUDE_CV_CLUSTERING_HPP_
#include <vector>
#include "protos/obc.pb.h"


class Clustering {
 public:
    std::vector<GPSCoord> FindClustersCenter(const std::vector<std::vector<GPSCoord>>& points);
};

#endif  // INCLUDE_CV_CLUSTERING_HPP_
